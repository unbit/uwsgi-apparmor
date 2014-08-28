uwsgi-apparmor
==============

uWSGI integration with apparmor

This plugin adds feature for applying apparmor profiles to uWSGI instances.

It requires uWSGI >= 2.0.7

Installation
============

The plugin is 2.x friendly

```sh
uwsgi --build-plugin https://github.com/unbit/uwsgi-apparmor
```

Usage
=====

The plugin exposes the following features:

* applying a profile to the instance with the `--apparmor-profile <name>` option
* setting apparmor profile from a custom hook with the `apparmor:name` action
* applying a profile to vassals before they are exec'ed
* (Only for uWSGI >= 2.1) applying a profile to vassals before they are exec'ed using emperor attributes (each vassal can have a different profile)

Example usage
=============

To apply a profile to an instance (remember to load the profile with `appamor_parser` command line tool)

```ini
[uwsgi]
plugin = apparmor
apparmor-profile = funnyprofile
; ensure your profile allow INET usage
socket = 127.0.0.1:3031
; ensure your profile allows write access to /run/foo.pid
pidfile = /run/foo.pid
; ensure your profile allows read access to /var/www/app.psgi
psgi = /var/www/app.psgi
```

To apply the profile as a hook:

```sh
uwsgi --plugin apparmor --hook-as-root apparmor:funnyprofile ...
```

And to apply it to every vassal

```ini
[uwsgi]
plugin = apparmor
emperor = vassals
; ensure vassalsprofile allows execution of uwsgi binary
emperor-apparmor = vassalsprofile
```

If you have uWSGI >= 2.1, you can use emperor attributes to set the apparmor profile:

```ini
[emperor]
; this is the profiel to apply to the vassal
apparmor = vassal001

[uwsgi]
; ensure your profile allow INET usage
socket = 127.0.0.1:3031
; ensure your profile allows write access to /run/foo.pid
pidfile = /run/foo.pid
; ensure your profile allows read access to /var/www/app.psgi
psgi = /var/www/app.psgi
```

then run the Emperor

```sh
uwsgi --plugin apparmor --emperor vassals --emperor-collect-attr apparmor --emperor-apparmor-attr apparmor
```

remember to specify which attr to collect and to use (yes it seems redundant, one day this could be improved ...)
