btest_add(note_parsing "parse_notes.cpp")
btest_add(sine_calculation "sine_calculation.cpp")
btest_add(note_frequencies "note_frequencies.cpp")
btest_add(sequence_engines "sequence_engines.cpp")
btest_add(sample_generator "sample_generator.cpp")
btest_add(settings SOURCES "settings.cpp" "../src/settings.cpp" LIBS "${BOOST_PROGOPT_LIB}")
