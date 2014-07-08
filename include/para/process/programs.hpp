// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Identifiers of a named program.
 */

#ifndef PARA_PROCESS_PROGRAM_HPP_p34u6pb5
#define PARA_PROCESS_PROGRAM_HPP_p34u6pb5

namespace para {
  namespace process {

    //! \name Binary Specification Tags
    //@{
    //! \ingroup grp_proc

    // yes, the ingroup really goes there.

    //! Use env()
    struct set_env_tag {};
    //! Don't use env()
    struct no_env_tag {};

    //! Search paths in the PATH environment variable
    struct path_bin_tag {};
    //! No search.  Just use pwd.
    struct exact_bin_tag {};
    //! Search like on windows (app dir, windows dirs, path, pwd)
    struct windows_bin_tag {};
    //! Path search or windows search.  This is not a typedef so that we can
    //! cause a compile failure when the other types are not implemented on
    //! *both* sytems.
    struct platform_bin_tag {};

    //@}

    namespace detail {
      //! Reduce the code complexity a little bit.  This is used for stuff that
      //! stores a pointer to a non-integral type (eg vector, string).
      template<class Data, class Returned>
      struct container_storage {
        container_storage() : val_(NULL) {}

        Returned get() { return val_ ? &(*val_)[0] : NULL; }
        void set(Data &d) { val_ = &d; }

        Data *val_;
      };

      //! Ditto.  This is used by stuff that stores a pointer to an integal type
      //! (eg. char*, char**).
      template<class Data>
      struct integral_storage {
        integral_storage() : val_(NULL) {}

        Data get() { return val_; }
        void set(Data d) { val_ = d; }

        Data val_;
      };

      //! This class and its specialisations allow us to transparently store
      //! pointers to containers, strings, c-strings, and arrays of c-strings.
      template<class Container>
      class arg_storage : protected container_storage<Container, typename Container::value_type*> {
        public:
          typedef container_storage<Container,typename Container::value_type*> base_type;
          typedef Container container_type;
          // just so I can get the typename in!!
          typedef typename container_type::value_type value_type;
          typedef value_type* get_type;
          typedef container_type& set_type;

          using base_type::set;
          using base_type::get;
      };

      template<>
      class arg_storage<std::string> : protected container_storage<std::string, char*> {
        public:
          typedef container_storage<std::string, char*> base_type;
          typedef char* get_type;
          typedef std::string& set_type;

          using base_type::get;
          using base_type::set;
      };

      template<>
      class arg_storage<char**>  : protected integral_storage<char**> {
        public:
          typedef integral_storage<char**> base_type;

          typedef char** get_type;
          typedef char** set_type;

          using base_type::set;
          using base_type::get;
      };

      template<>
      class arg_storage<char*> : protected integral_storage<char*> {
        public:
          typedef integral_storage<char*> base_type;

          typedef char* get_type;
          typedef char* set_type;

          using base_type::set;
          using base_type::get;
      };

    }

    //! Base type for differently tagged binaries.
    template<class Container>
    class binary_spec_base {
      public:
        typedef typename detail::arg_storage<Container>::get_type get_type;
        typedef typename detail::arg_storage<Container>::set_type set_type;

        binary_spec_base(const char *path) : path_(path) {}

        const char *path() const { return path_; }

        void args(set_type a) { args_.set(a); }
        get_type args() const { return args_.get(); }

      private:
        const char *path_;
        detail::arg_storage<Container> args_;
    };

    //! Base class dealing with representaition of an environment list.
    template<class Container>
    class environment_spec_base {
      public:
        typedef typename detail::arg_storage<Container>::get_type get_type;
        typedef typename detail::arg_storage<Container>::set_type set_type;

        environment_spec_base() {}

        get_type envs() const { return envs_.get(); }
        void envs(set_type e) { return envs_.set(e); }

      private:
        detail::arg_storage<Container> envs_;
    };

    //! \ingroup grp_proc
    //! Using platform_bin_tag.
    template<class Container = char **>
    class platform_binary : public binary_spec_base<Container> {
      public:
        typedef platform_bin_tag bin_tag;
        typedef no_env_tag env_tag;

        platform_binary(const char *name) : binary_spec_base<Container>(name) {}
    };

    //! \ingroup grp_proc
    //! Adds an environment array to a binary details.
    template<class BinaryDetails>
    class environment_set :
      public BinaryDetails, public environment_spec_base<typename BinaryDetails::container_type>
    {
      typedef environment_spec_base<typename BinaryDetails::container_type> env_spec_type;

      public:
        typedef typename BinaryDetails::bin_tag bin_tag;
        typedef set_env_tag env_tag;
    };

    /**** old stuff below this ****/

    //! BinaryDetails template tag class (see named_process).
    struct no_search_no_env_tag {};

    //! \ingroup grp_proc
    //! BinaryDetails template tag class (see named_process).
    struct no_search_env_tag {};

    //! \ingroup grp_proc
    //! BinaryDetails template tag class (see named_process).
    struct search_no_env_tag {};

    //! \ingroup grp_proc
    //! Binary details with a reference to a c++ container for argv
    template<class Container, class DetailsTag>
    struct binary {
      public:
        typedef DetailsTag details_tag;

        binary(const char *path) : path_(path), args_(0), env_(0) { }

        const char *path() const { return path_; }

        binary &args(Container &args) { set_args(&args); return *this; }
        char **args() const { return (get_args()) ? (*get_args())[0] : NULL; }

        binary &env(Container &env) { set_env(env); return *this; }
        char **env() const { return (get_env()) ? &(*get_env())[0] : NULL; }

      protected:
        Container *get_args() { return args_; }
        void set_args(Container *a) { return args_ = a; }
        Container *get_env() { return env_; }
        void set_env(Container *e) { return env_ = e; }

      private:
        const char *path_;
        Container *args_;
        // this is useless most of the time
        Container *env_;
    };

    //! \ingroup grp_proc
    //! Specialisation allowing for a c-array.
    //TODO:
    //  maybe it should protected-interit binary<char* [sic],DetailsTag> and
    //  override args() and/or env()
    template<class DetailsTag>
    class binary<char**,DetailsTag> {
      public:
        typedef DetailsTag details_tag;

        binary(const char *path) : path_(path), args_(0), env_(0) { }

        const char *path() const { return path_; }

        binary &args(char **args) { args_ = args; return *this; }
        char **args() const { return args_; }

        binary &env(char **env) { env_ = env; return *this; }
        char **env() const { return env_;}

      private:
        const char *path_;
        char **args_;
        char **env_;
    };

    //! \name Convenience wrappers for class binary.
    //! This avoids having to remember the tag classes.
    //@{

    //! \ingroup grp_proc
    //! Search in path if not string contains '/'.
    template<class Container = char**>
    struct path_program : public binary<Container, search_no_env_tag> {
      typedef binary<Container, search_no_env_tag> base_type;
      path_program(const char *p) : base_type(p) {}
    };

    //! \ingroup grp_proc
    //! No search in path, use environment.
    template<class Container = char**>
    struct env_program : public binary<Container, no_search_env_tag> {
      typedef binary<Container, no_search_env_tag> base_type;
      env_program(const char *p) : base_type(p) {}
    };

    //! \ingroup grp_proc
    //! No search in path.
    template<class Container = char**>
    struct program : public binary<Container, no_search_no_env_tag> {
      typedef binary<Container, no_search_no_env_tag> base_type;
      program(const char *p) : base_type(p) {}
    };
    //@}

  }
}
#endif
