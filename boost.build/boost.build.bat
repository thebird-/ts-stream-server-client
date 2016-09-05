pushd %BoostDir%
b2 -j8 toolset=msvc-14.0 address-model=%BoostAddressModel% architecture=x86 link=static threading=multi runtime-link=static --build-type=minimal stage --stagedir=%BoostLibDirName% --with-program_options --with-system --with-date_time --with-regex --with-context --with-coroutine
popd