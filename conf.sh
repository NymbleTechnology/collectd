#!/bin/bash
#
# configures collectd for cross-compilation
#
set -x

source ../setup-cross.inc

OPTIONS="--build=$BUILD --host=$TARGET"
OPTIONS="$OPTIONS --prefix=/usr --sysconfdir=/etc --localstatedir=/var --datadir=/mnt/dataflash"
OPTIONS="$OPTIONS --disable-static --without-perl-bindings --disable-perl --disable-python --disable-java"
OPTIONS="$OPTIONS --with-fp-layout=nothing"

./configure $OPTIONS

# To assign environment variables (e.g., CC, CFLAGS...), specify them as
# VAR=VALUE.  See below for descriptions of some of the useful variables.
#
# Defaults for the options are specified in brackets.
#
# Configuration:
#  -h, --help              display this help and exit
#      --help=short        display options specific to this package
#      --help=recursive    display the short help of all the included packages
#  -V, --version           display version information and exit
#  -q, --quiet, --silent   do not print `checking ...' messages
#      --cache-file=FILE   cache test results in FILE [disabled]
#  -C, --config-cache      alias for `--cache-file=config.cache'
#  -n, --no-create         do not create output files
#      --srcdir=DIR        find the sources in DIR [configure dir or `..']
#
# System types:
#   --build=BUILD     configure for building on BUILD [guessed]
#   --host=HOST       cross-compile to build programs to run on HOST [BUILD]
#
# Installation directories:
#  --exec-prefix=EPREFIX
# install architecture-dependent files in EPREFIX [PREFIX]
#
# By default, `make install' will install all the files in
# `/opt/collectd/bin', `/opt/collectd/lib' etc.  You can specify
# an installation prefix other than `/opt/collectd' using `--prefix',
# for instance `--prefix=$HOME'.
#
# For better control, use the options below.
#
# Fine tuning of the installation directories:
#   --bindir=DIR            user executables [EPREFIX/bin]
#   --sbindir=DIR           system admin executables [EPREFIX/sbin]
#   --libexecdir=DIR        program executables [EPREFIX/libexec]
#   --sharedstatedir=DIR    modifiable architecture-independent data [PREFIX/com]
#   --localstatedir=DIR     modifiable single-machine data [PREFIX/var]
#   --libdir=DIR            object code libraries [EPREFIX/lib]
#   --includedir=DIR        C header files [PREFIX/include]
#   --oldincludedir=DIR     C header files for non-gcc [/usr/include]
#   --datarootdir=DIR       read-only arch.-independent data root [PREFIX/share]
#   --datadir=DIR           read-only architecture-independent data [DATAROOTDIR]
#   --infodir=DIR           info documentation [DATAROOTDIR/info]
#   --localedir=DIR         locale-dependent data [DATAROOTDIR/locale]
#   --mandir=DIR            man documentation [DATAROOTDIR/man]
#   --docdir=DIR            documentation root [DATAROOTDIR/doc/collectd]
#   --htmldir=DIR           html documentation [DOCDIR]
#   --dvidir=DIR            dvi documentation [DOCDIR]
#   --pdfdir=DIR            pdf documentation [DOCDIR]
#   --psdir=DIR             ps documentation [DOCDIR]
#
# Program names:
#   --program-prefix=PREFIX            prepend PREFIX to installed program names
#   --program-suffix=SUFFIX            append SUFFIX to installed program names
#   --program-transform-name=PROGRAM   run sed PROGRAM on installed program names
#
# Optional Features:
#   --disable-option-checking  ignore unrecognized --enable/--with options
#   --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
#   --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
#   --enable-shared[=PKGS]  build shared libraries [default=yes]
#   --enable-static[=PKGS]  build static libraries [default=yes]
#   --enable-fast-install[=PKGS]
#                           optimize for fast installation [default=yes]
#   --disable-libtool-lock  avoid locking (might break parallel builds)
#   --enable-ltdl-install   install libltdl
#   --enable-dependency-tracking
#                           do not reject slow dependency extractors
#   --disable-dependency-tracking
#                           speeds up one-time build
#   --enable-silent-rules   less verbose build output (undo: "make V=1")
#   --disable-silent-rules  verbose build output (undo: "make V=0")
#   --disable-largefile     omit support for large files
#   --enable-standards      Enable standards compliance mode
#
# collectd features:
#   --enable-debug          enable debugging (disabled by def)
#   --disable-daemon        disable daemon mode (enabled by def)
#   --enable-getifaddrs     enable getifaddrs under Linux (disabled by def)
#   --disable-werror        disable building with -Werror (enabled by def)
#
# collectd plugins:
#   --enable-all-plugins    enable all plugins (auto by def)
#
#   --enable-aggregation    Aggregation plugin
#   --enable-amqp           AMQP output plugin
#   --enable-apache         Apache httpd statistics
#   --enable-apcups         Statistics of UPSes by APC
#   --enable-apple_sensors  Apple's hardware sensors
#   --enable-aquaero        Aquaero's hardware sensors
#   --enable-ascent         AscentEmu player statistics
#   --enable-barometer      Barometer sensor on I2C
#   --enable-battery        Battery statistics
#   --enable-bind           ISC Bind nameserver statistics
#   --enable-ceph           Ceph daemon statistics
#   --enable-conntrack      nf_conntrack statistics
#   --enable-contextswitch  context switch statistics
#   --enable-cpufreq        CPU frequency statistics
#   --enable-cpu            CPU usage statistics
#   --enable-csv            CSV output plugin
#   --enable-curl           CURL generic web statistics
#   --enable-curl_json      CouchDB statistics
#   --enable-curl_xml       CURL generic xml statistics
#   --enable-cgroups        CGroups CPU usage accounting
#   --enable-dbi            General database statistics
#   --enable-df             Filesystem usage statistics
#   --enable-disk           Disk usage statistics
#   --enable-drbd           DRBD statistics
#   --enable-dns            DNS traffic analysis
#   --enable-email          EMail statistics
#   --enable-entropy        Entropy statistics
#   --enable-ethstat        Stats from NIC driver
#   --enable-exec           Execution of external programs
#   --enable-fhcount        File handles statistics
#   --enable-filecount      Count files in directories
#   --enable-fscache        fscache statistics
#   --enable-gmond          Ganglia plugin
#   --enable-hddtemp        Query hddtempd
#   --enable-interface      Interface traffic statistics
#   --enable-ipc            IPC statistics
#   --enable-ipmi           IPMI sensor statistics
#   --enable-iptables       IPTables rule counters
#   --enable-ipvs           IPVS connection statistics
#   --enable-irq            IRQ statistics
#   --enable-java           Embed the Java Virtual Machine
#   --enable-load           System load
#   --enable-logfile        File logging plugin
#   --enable-log_logstash   Logstash json_event compatible logging
#   --enable-lpar           AIX logical partitions statistics
#   --enable-lvm            LVM statistics
#   --enable-madwifi        Madwifi wireless statistics
#   --enable-match_empty_counter
#                           The empty counter match
#   --enable-match_hashed   The hashed match
#   --enable-match_regex    The regex match
#   --enable-match_timediff The timediff match
#   --enable-match_value    The value match
#   --enable-mbmon          Query mbmond
#   --enable-md             md (Linux software RAID) devices
#   --enable-memcachec      memcachec statistics
#   --enable-memcached      memcached statistics
#   --enable-memory         Memory usage
#   --enable-mic            Intel Many Integrated Core stats
#   --enable-modbus         Modbus plugin
#   --enable-mqtt           MQTT output plugin
#   --enable-multimeter     Read multimeter values
#   --enable-mysql          MySQL statistics
#   --enable-netapp         NetApp plugin
#   --enable-netlink        Enhanced Linux network statistics
#   --enable-network        Network communication plugin
#   --enable-nfs            NFS statistics
#   --enable-nginx          nginx statistics
#   --enable-notify_desktop Desktop notifications
#   --enable-notify_email   Email notifier
#   --enable-notify_nagios  Nagios notification plugin
#   --enable-ntpd           NTPd statistics
#   --enable-numa           NUMA virtual memory statistics
#   --enable-nut            Network UPS tools statistics
#   --enable-olsrd          olsrd statistics
#   --enable-onewire        OneWire sensor statistics
#   --enable-openldap       OpenLDAP statistics
#   --enable-openvpn        OpenVPN client statistics
#   --enable-oracle         Oracle plugin
#   --enable-perl           Embed a Perl interpreter
#   --enable-pf             BSD packet filter (PF) statistics
#   --enable-pinba          Pinba statistics
#   --enable-ping           Network latency statistics
#   --enable-postgresql     PostgreSQL database statistics
#   --enable-powerdns       PowerDNS statistics
#   --enable-processes      Process statistics
#   --enable-protocols      Protocol (IP, TCP, ...) statistics
#   --enable-python         Embed a Python interpreter
#   --enable-redis          Redis plugin
#   --enable-routeros       RouterOS plugin
#   --enable-rrdcached      RRDTool output plugin
#   --enable-rrdtool        RRDTool output plugin
#   --enable-sensors        lm_sensors statistics
#   --enable-serial         serial port traffic
#   --enable-sigrok         sigrok acquisition sources
#   --enable-smart          SMART statistics
#   --enable-snmp           SNMP querying plugin
#   --enable-statsd         StatsD plugin
#   --enable-swap           Swap usage statistics
#   --enable-syslog         Syslog logging plugin
#   --enable-table          Parsing of tabular data
#   --enable-tail           Parsing of logfiles
#   --enable-tail_csv       Parsing of CSV files
#   --enable-tape           Tape drive statistics
#   --enable-target_notification
#                           The notification target
#   --enable-target_replace The replace target
#   --enable-target_scale   The scale target
#   --enable-target_set     The set target
#   --enable-target_v5upgrade
#                           The v5upgrade target
#   --enable-tcpconns       TCP connection statistics
#   --enable-teamspeak2     TeamSpeak2 server statistics
#   --enable-ted            Read The Energy Detective values
#   --enable-thermal        Linux ACPI thermal zone statistics
#   --enable-threshold      Threshold checking plugin
#   --enable-tokyotyrant    TokyoTyrant database statistics
#   --enable-turbostat      Advanced statistic on Intel cpu states
#   --enable-unixsock       Unixsock communication plugin
#   --enable-uptime         Uptime statistics
#   --enable-users          User statistics
#   --enable-uuid           UUID as hostname plugin
#   --enable-varnish        Varnish cache statistics
#   --enable-virt           Virtual machine statistics
#   --enable-vmem           Virtual memory statistics
#   --enable-vserver        Linux VServer statistics
#   --enable-wireless       Wireless statistics
#   --enable-write_graphite Graphite / Carbon output plugin
#   --enable-write_http     HTTP output plugin
#   --enable-write_kafka    Kafka output plugin
#   --enable-write_log      Log output plugin
#   --enable-write_mongodb  MongoDB output plugin
#   --enable-write_redis    Redis output plugin
#   --enable-write_riemann  Riemann output plugin
#   --enable-write_sensu    Sensu output plugin
#   --enable-write_tsdb     TSDB output plugin
#   --enable-xmms           XMMS statistics
#   --enable-zfs_arc        ZFS ARC statistics
#   --enable-zone           Solaris container statistics
#   --enable-zookeeper      Zookeeper statistics
#
# Optional Packages:
#   --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
#   --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
#   --with-pic[=PKGS]       try to use only PIC/non-PIC objects [default=use
#                           both]
#   --with-gnu-ld           assume the C compiler uses GNU ld [default=no]
#   --with-sysroot=DIR Search for dependent libraries within DIR
#                         (or the compiler's sysroot if not specified).
#   --with-included-ltdl    use the GNU ltdl sources included here
#   --with-ltdl-include=DIR use the ltdl headers installed in DIR
#   --with-ltdl-lib=DIR     use the libltdl.la installed in DIR
#   --with-nan-emulation    use emulated NAN. For crosscompiling only.
#   --with-fp-layout        set the memory layout of doubles. For crosscompiling
#                           only.
#   --with-useragent[=AGENT]
#                           User agent to use on http requests
#   --with-libpthread=[=PREFIX]
#                           Path to libpthread.
#
# collectd additional packages:
#   --with-libaquaero5[=PREFIX]
#                           Path to aquatools-ng source code.
#   --with-libhiredis[=PREFIX]
#                           Path to libhiredis.
#   --with-libcurl[=PREFIX] Path to libcurl.
#   --with-libdbi[=PREFIX]  Path to libdbi.
#   --with-libesmtp[=PREFIX]
#                           Path to libesmtp.
#   --with-libganglia[=PREFIX]
#                           Path to libganglia.
#   --with-libgcrypt[=PREFIX]
#                           Path to libgcrypt.
#   --with-libiptc[=PREFIX] Path to libiptc.
#   --with-java[=PREFIX]    Path to Java home.
#   --with-libldap[=PREFIX] Path to libldap.
#   --with-liblvm2app[=PREFIX]
#                           Path to liblvm2app.
#   --with-libmemcached[=PREFIX]
#                           Path to libmemcached.
#   --with-libmodbus[=PREFIX]
#                           Path to the modbus library.
#   --with-libmongoc[=PREFIX]
#                           Path to libmongoc.
#   --with-libmosquitto[=PREFIX]
#                           Path to libmosquitto.
#   --with-libmysql[=PREFIX]
#                           Path to libmysql.
#   --with-libmnl[=PREFIX]  Path to libmnl.
#   --with-libnetapp[=PREFIX]
#                           Path to libnetapp.
#   --with-libnetsnmp[=PREFIX]
#                           Path to the Net-SNMPD library.
#   --with-liboconfig[=PREFIX]
#                           Path to liboconfig.
#   --with-liboping[=PREFIX]
#                           Path to liboping.
#   --with-oracle[=ORACLE_HOME]
#                           Path to Oracle.
#   --with-libowcapi[=PREFIX]
#                           Path to libowcapi.
#   --with-libpcap[=PREFIX] Path to libpcap.
#   --with-libperl[=PREFIX] Path to libperl.
#   --with-libpq[=PREFIX]   Path to libpq.
#   --with-python[=PREFIX]  Path to the python interpreter.
#   --with-librabbitmq[=PREFIX]
#                           Path to librabbitmq.
#   --with-librdkafka[=PREFIX]
#                           Path to librdkafka.
#   --with-librouteros[=PREFIX]
#                           Path to librouteros.
#   --with-librrd[=PREFIX]  Path to rrdtool.
#   --with-libsensors[=PREFIX]
#                           Path to lm_sensors.
#   --with-libsigrok[=PREFIX]
#                           Path to libsigrok.
#   --with-libstatgrab[=PREFIX]
#                           Path to libstatgrab.
#   --with-libtokyotyrant[=PREFIX]
#                           Path to libtokyotyrant.
#   --with-libudev[=PREFIX] Path to libudev.
#   --with-libupsclient[=PREFIX]
#                           Path to the upsclient library.
#   --with-libxmms[=PREFIX] Path to libxmms.
#   --with-libyajl[=PREFIX] Path to libyajl.
#   --with-mic[=PREFIX]     Path to Intel MIC Access API.
#   --with-libvarnish[=PREFIX]
#                           Path to libvarnish.
#   --with-libatasmart[=PREFIX]
#                           Path to libatasmart.
#   --with-perl-bindings[=OPTIONS]
#                           Options passed to "perl Makefile.PL".
#
# Some influential environment variables:
#   CC          C compiler command
#   CFLAGS      C compiler flags
#   LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a
#               nonstandard directory <lib dir>
#   LIBS        libraries to pass to the linker, e.g. -l<library>
#   CPPFLAGS    (Objective) C/C++ preprocessor flags, e.g. -I<include dir> if
#               you have headers in a nonstandard directory <include dir>
#   CPP         C preprocessor
#   YACC        The `Yet Another Compiler Compiler' implementation to use.
#               Defaults to the first program found out of: `bison -y', `byacc',
#               `yacc'.
#   YFLAGS      The list of arguments that will be passed by default to $YACC.
#               This script will default YFLAGS to the empty string to avoid a
#               default value of `-d' given by some make applications.
#   PKG_CONFIG  path to pkg-config utility
#   PKG_CONFIG_PATH
#               directories to add to pkg-config's search path
#   PKG_CONFIG_LIBDIR
#               path overriding pkg-config's built-in search path
#   KERNEL_DIR  path to Linux kernel sources
#   LIBNETAPP_CPPFLAGS
#               C preprocessor flags required to build with libnetapp
#   LIBNETAPP_LDFLAGS
#               Linker flags required to build with libnetapp
#   LIBNETAPP_LIBS
#               Other libraries required to link against libnetapp
#   LIBNOTIFY_CFLAGS
#               C compiler flags for LIBNOTIFY, overriding pkg-config
#   LIBNOTIFY_LIBS
#               linker flags for LIBNOTIFY, overriding pkg-config
#
# Use these variables to override the choices made by `configure' or to help
# it to find libraries and programs with nonstandard names/locations.
#
# Report bugs to the package provider.
