#ifndef NO_FRACTION_STATS

    // initialize tpie subsystems and begin tracking progress
    tpie_init(ALL | CAPTURE_FRACTIONS);
    load_fractions("fractiondb.gen.inl");

    go();

    save_fractions("fractiondb.gen.inl");
    tpie_finish(ALL | CAPTURE_FRACTIONS);

#else

    tpie_init();
#include "fractiondb.gen.inl"

    go();

    tpie_finish();

#endif
