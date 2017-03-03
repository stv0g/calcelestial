# calcelestial [![Build Status](https://travis-ci.org/stv0g/calcelestial.svg?branch=master)](https://travis-ci.org/stv0g/calcelestial)

calcelestial calculates the sun's rise/set times, the solar noon and the daylight time duration.

See manpage calcelestial(1) and the [blog entry](https://www.noteblok.net/2012/12/23/cron-jobs-fur-sonnenauf-untergang/) for more information

# Installation

### Linux

```
sudo apt-get install -y libnova-dev libcurl4-openssl-dev libjson-c-dev libdb-dev autoconf make gcc pkg-config
autoreconf -i && ./configure && make install
```

### macOS

```
brew install curl json-c berkeley-db pkg-config
git clone git://git.code.sf.net/p/libnova/libnova libnova && pushd libnova && autoreconf -if && ./configure && make && sudo make install; popd
autoreconf -i && ./configure && make install
```

# Usage

```
Usage:
  Calcelestial [options]

Options:
  -p, --object		calc for celestial object: sun, moon, mars, neptune,
			 jupiter, mercury, uranus, saturn, venus or pluto
  -H, --horizon		calc rise/set time with twilight: nautic, civil or astronomical
  -t, --time		calc at given time: YYYY-MM-DD[_HH:MM:SS]
  -m, --moment		calc position at moment of: rise, set, transit
  -n, --next		use rise, set, transit time of tomorrow
  -f, --format		output format: see strftime (3) and calcelestial (1) for more details
  -a, --lat		geographical latitude of observer: -90° to 90°
  -o, --lon		geographical longitude of oberserver: -180° to 180°
  -q, --query		query coordinates using the geonames.org geolocation service
  -l, --local		query local timezone using the geonames.org geolocation service
  -z, --timezone	override system timezone (TZ environment variable)
  -u, --universal	use universial time for parsing and formatting
  -h, --help		show usage help
  -v, --version		show version

Note: A combination of --lat & --lon or --query is required.

The following special tokens are supported in the --format parameter:

  §J	Julian date of observation
  §d	Diameter in arc seconds
  §e	Distance to object in astronomical units
  §r	Equatorial Coordinates: Right Ascension in degrees
  §d	Equatorial Coordinates: Declincation in degrees
  §a	Horizontal Coordinates: Azimuth in degrees
  §h	Horizontal Coordinates: Altitude in degrees
  §A	Latitude in degrees
  §O	Longitude in degrees
  §s	Azimuth direction (N, E, S, W, NE, ...)

Calcelestial is written by Steffen Vogel <post@steffenvogel.de>
Please report bugs to: https://github.com/stv0g/calcelestial/issues
```

## Examples

The simplest variant uses the Unix tool [at(1)](https://linux.die.net/man/1/at) to schedule a command at a specific event:

```
echo ~/bin/turn-lights-on | at $(calcelestial -p sun -m set -q Frankfurt -H civil)
```

The following cronjobs automate this task everyday with updated times:

```
0 0 * * * echo 'fnctl stop && fnctl fade -c 000000' | at $(calcelestial -m rise -p sun -q Aachen)
0 0 * * * echo 'fnctl start' | at $(calcelestial -m set -p sun -q Frankfurt)
```

The tool [nvram-wakeup](http://www.vdr-wiki.de/wiki/index.php/NVRAM_WakeUp), can be used to turn on the system everyday 10 minutes before sunrise in Berlin:

```
nvram-wakeup -s $(date -d "$(calcelestial -m rise -p sun -q Berlin --format %+)" -10 minutes" +%s)
```

Or alternatively, the system can be shut down 10 minutes after sunset:

```
shutdown $(date -d "$(calcelestial -m rise -p sun --lat=50.55 --lon=-6.2 --format %+) +10 minutes" +%H:%M)
```

The current position of the moon can be estimated with:

```
calcelestial -p moon -q Aachen -f "az: §a alt: §h"
```

# License

calcelestial is licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).

# Author

calcelestial is written by Steffen Vogel <post@steffenvogel.de>
