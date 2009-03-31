/*!
\file
\brief Basis for SDL library
*/

#ifndef SDL_HPP_uyqa3zsm
#define SDL_HPP_uyqa3zsm

// this must be defined before this is included
#ifdef HAVE_SDL_H
// Only on really old versions?
#  include <SDL.h>
#else
// assume this because it's the normal way
#  include <SDL/SDL.h>
#endif

#include <cassert>
#include <stdexcept>
#include <ostream>
#include <cstring>


/*
http://www.libsdl.org/cgi/docwiki.cgi/SDL_API - scroll down to 'Audio'

http://www.libsdl.org/cgi/docwiki.cgi/Audio_Examples

// Note: this will be too slow and not good enough quality for my purposes - use OpenAL or
// SDL_mixer, or my own one.  I need an API for it also.
SDL_MixAudio - Mixes audio data
*/

namespace sdl {

//! \brief Convenience base for all errors; what() is SDL_GetError unless specified in ctor
struct error : public std::runtime_error {
  error(const char *e = NULL) : runtime_error(((e == NULL) ? SDL_GetError() : e)) {}
};

//!\deprecated Use the type directly
typedef error sdl_error;

//! \brief SDL_Init failure.
struct init_error : public error {
  init_error(const char *e = NULL) : error(e) {}
};

//! \brief SDL_OpenAudio failure.
struct open_error : public error {
  open_error(const char *e = NULL) : error(e) {}
};

//! \brief RAII object for SDL_Init and SDL_quit and interface for static audio stuff.
class audio {
  public:
    audio() {
      int r = SDL_Init(SDL_INIT_AUDIO);
      if (r != 0) {
        const char * const error_str = SDL_GetError();
        assert(error_str != NULL);
        throw init_error(error_str);
      }
    }

    ~audio() {
      SDL_Quit();
    }

    //! \brief stopped, playing, or paused.
    SDL_audiostatus status() const { return SDL_GetAudioStatus(); }

    const char *status_name(SDL_audiostatus s) const {
      switch (s) {
        case SDL_AUDIO_STOPPED:
          return "stopped";
        case SDL_AUDIO_PLAYING:
          return "playing";
        case SDL_AUDIO_PAUSED:
          return "paused";
        default:
          throw std::logic_error("unpossible value for SDL_GetAudioStatus().");
      }
    }

    const char *status_name() const { return status_name(status()); }
};


//! \brief C++ wrapper over SDL_AudioSpec.
class audio_spec {
  friend class device_base;

// TODO:
//   if we refuse nonconst access to bytes and the other silence then
//   we can pass this struct around const (given a mutable sdl struct)

  public:
    typedef void(*callback_type)(void *user_data, Uint8 *stream, int len);

    typedef enum {defer_init} defer_init_type;

    //! \brief Defer initialisation until later - intended for the obtained value output, but could be useful.
    explicit audio_spec(defer_init_type) {}

    // TODO: what exactly is the data.
    explicit audio_spec(callback_type callback, int freq = 44100, int samples = 1024, int channels = 2, int format = AUDIO_S16SYS) {
      // std::memset(spec(), 0, sizeof(SDL_AudioSpec));
      spec().freq = freq;
      spec().format = format;
      spec().callback = callback;
      spec().channels = channels;
      spec().samples = samples;
      spec().userdata = NULL;
      spec().silence = 0;
      spec().size = 0;
    }

    //! \brief Mono = 1, stereo = 2, etc.
    int channels() const { return spec().channels; }
    void channels(int n) { spec().channels = n; }

    //! \brief Size in bytes of the buffer (calculated from sizeof(data) * samples() * channels()).
    std::size_t buffer_size() const { return spec().size; }

    //! \brief Buffer size in samples.
    int buffer_samples() const { return spec().samples; }

    //! \brief Audio sample rate.
    uint32_t frequency() const { return spec().freq; }
    uint32_t frequency(uint32_t f) { return spec().freq = f; }

    //! \brief This can be something other than 0 if you're not using signed audio!
    int silence() const { return spec().silence; }

    //! \brief Time period in miliseconds of the buffer calculated by ms = (buffer_samples() * 1000) / frequency().
    int period() const { return (buffer_samples() * 1000 ) / frequency(); }

    friend std::ostream &operator<<(std::ostream &o, const audio_spec &a) {
      a.dump_spec(o);
      return o;
    }

    // \brief Only tests non-calculated functions and *does* include userdata.
    friend bool operator==(const audio_spec &lhs, const audio_spec &rhs) {
      return
        lhs.spec().freq == rhs.spec().freq &&
        lhs.spec().format == rhs.spec().format &&
        lhs.spec().callback == rhs.spec().callback &&
        lhs.spec().channels == rhs.spec().channels &&
        lhs.spec().samples == rhs.spec().samples &&
        lhs.spec().userdata == rhs.spec().userdata;
    }

    friend bool operator!=(const audio_spec &lhs, const audio_spec &rhs) {
      return ! (rhs == lhs);
    }

    //! \deprecated Use dump()
    std::ostream &dump_spec(std::ostream &o, const char *prefix = "") const { return dump(o, prefix); }

    //! \brief like operator<< but you can indent it.  Doesn't have a newline terminating.
    std::ostream &dump(std::ostream &o, const char *prefix = "") const {
      const SDL_AudioSpec &s = spec();

      o << prefix << "Frequency:         " << s.freq << "\n";
      o << prefix << "Format:            ";
      switch (s.format) {
        case AUDIO_U8:
          o << "u8"; break;
        case AUDIO_S8:
          o << "s8"; break;
        case AUDIO_U16LSB:
          assert(AUDIO_U16LSB == AUDIO_U16);
          o << "u16 little"; break;
        case AUDIO_S16LSB:
          assert(AUDIO_S16LSB == AUDIO_S16);
          o << "s16 little"; break;
        case AUDIO_U16MSB:
          o << "u16 big"; break;
        case AUDIO_S16MSB:
          o << "s16 big"; break;
        default:
          o << "(unexpected value)";
      }
      o << "\n";

      o << prefix << "Channels:          " << (int) s.channels << "\n";
      o << prefix << "Silence value:     " << (int) s.silence << "\n";
      o << prefix << "Buffer in samples: " << s.samples << "\n";
      o << prefix << "Buffer in bytes:   " << s.size << "\n";
      o << prefix << "Buffer in ms:      " << period() << "\n";
      o << prefix << "Callback:          " << (void*) s.callback << "\n";
      o << prefix << "Userdata:          " << (void*) s.userdata;
      return o;
    }

  private:
    SDL_AudioSpec spec_;

    SDL_AudioSpec &spec() { return spec_; }
    const SDL_AudioSpec &spec() const { return spec_; }
};

//! \brief Open/close the audio device; non-instancable base class.
class device_base {
  public:
    /*!
    \brief Change the pause state.  Silence is written when paused.

    It should be called with pause_on=0 after opening the audio device to start playing sound.
    This is so you can safely initialize data for your callback function after opening the audio
    device. Silence will be written to the audio device during the pause.
    */
    //@{
    void pause() { pause_state(1); }
    void unpause() { pause_state(0); }
    void pause_state(int on) { SDL_PauseAudio(on); }
    //@}

  protected:
    device_base(sdl::audio &) {}

    //! \brief Close the audio and stop outputting
    ~device_base() { SDL_CloseAudio(); }

    // TODO: what happens in case of a double open?
    void checked_open(audio_spec &des, audio_spec &obt) {
      checked_open(&des.spec(),  &obt.spec());
    }

    void checked_open(audio_spec &des) {
      checked_open(&des.spec());
    }

  private:
    // "SDL_OpenAudio calculates the size and silence fields for both the desired and
    // obtained specifications. The size field stores the total size of the audio buffer
    // in bytes, while the silence stores the value used to represent silence in the
    // audio buffer" - that's why it's nonconst.
    void checked_open(SDL_AudioSpec *des, SDL_AudioSpec *obt = NULL) {
      int r = SDL_OpenAudio(des, obt);
      if (r != 0) {
        throw open_error();
      }
      assert(SDL_GetAudioStatus() == SDL_AUDIO_PAUSED);
    }
};

//! \brief Open/close the audio device; don't store any info.
class light_device : public device_base {
  public:
    //! \brief Doesn't initialise so the desired audio_spec can go out of scope.
    light_device(sdl::audio &a) : device_base(a) {}
    //! \brief Call reopen() immediately (throws open_error) .
    light_device(sdl::audio &a, audio_spec &desired) : device_base(a) {
      reopen(desired);
    }
    //! \brief Call reopen() immediately (throws open_error) .
    light_device(sdl::audio &a, audio_spec &desired, audio_spec &obtained) : device_base(a) {
      reopen(desired, obtained);
    }

    //! \brief Open audio device with the requested spec (might get something different).  Status will be paused.
    //! Probably better to use \link device \endlink if you are using this method.
    void reopen(audio_spec &desired, audio_spec &obtained) {
      checked_open(desired, obtained);
    }

    //! \brief Open audio device with the requested spec (might get something different).  Status will be paused.
    void reopen(audio_spec &desired) {
      checked_open(desired);
    }
};

//! \brief Open the audio output; stores properties of what we opened.
class device : public device_base {
  public:
    //! \brief Doesn't initialise so the desired audio_spec can go out of scope.
    device(sdl::audio &a) : device_base(a), obtained_(audio_spec::defer_init) {}
    //! \brief Call reopen() immediately (throws open_error) .
    device(sdl::audio &a, audio_spec &desired) : device_base(a), obtained_(audio_spec::defer_init) {
      checked_open(desired, obtained_);
    }

    //! \brief Accessor to what was actually opened.
    const audio_spec &obtained() const { return obtained_; }

    //! \brief Open audio device with the requested spec (might get something different).  Status will be paused.
    //! Desired is non-const because there are some values which are calculated.  See
    //! \link audio_spec \endlink for details.
    void reopen(audio_spec &desired) {
      checked_open(desired, obtained_);
    }

  private:
    audio_spec obtained_;
};

//! \brief Prevent the callback function from being called.
class audio_lock_guard {
  public:
    //! \brief Call SDL_LockAudio().  Parameter is there to ensures that we are set up.
    audio_lock_guard(const device_base &) { SDL_LockAudio(); }
    //! \brief Call SDL_UnlockAudio()
    ~audio_lock_guard() { SDL_UnlockAudio(); }
};

} // ns sdl

#endif
