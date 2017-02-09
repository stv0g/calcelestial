# calcelestial [![Build Status](https://travis-ci.org/stv0g/calcelestial.svg?branch=master)](https://travis-ci.org/stv0g/calcelestial)

calcelestial calculates the sun's rise/set times, the solar noon and the daylight time duration.

See manpage calcelestial(1) and the [blog entry](https://www.noteblok.net/2012/12/23/cron-jobs-fur-sonnenauf-untergang/) for more information

# Installation

### Linux

```
sudo apt-get install -y libnova-dev libcurl4-openssl-dev libjson-c-dev libdb-dev autoconf
autoreconf -i && ./configure && make install
````
### macOS

```
brew install curl json-c berkeley-db
git clone git://git.code.sf.net/p/libnova/libnova libnova && pushd libnova && autoreconf -if && ./configure && make && sudo make install; popd
autoreconf -i && ./configure && make install
```

# License

calcelestial is licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).

# Author

calcelestial is written by Steffen Vogel <post@steffenvogel.de>
