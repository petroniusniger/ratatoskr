# Sprint #007: Investigate device list display issue
(pan, v0, 29/01/2026)


## Source of "Bluetooth" string displayed on SharePlugin screen

Only instance of the single word "Bluetooth" in the SharePlugin source tree is:

```
./shareplugin/shareplugin.desktop:Name=Bluetooth
```

Renamed it to "BT Plugin" for clarity. Text in middle of back background 
window reflected the change.

## Is the CPP executable actually loaded?

Desktop file contains the following:

```
[Desktop Entry]
Name=Bluetooth
Exec=shareplugin
Icon=ratatoskr
Terminal=false
Type=Application
X-Ubuntu-Touch=true
X-Lomiri-Single-Instance=true
```

Executable name defined in `CMakeLists.txt`:

```
set(SHAREPLUGIN_NAME "shareplugin")
```

Conclusion: no reason to doubt that it is.

## Why can't the plugin display anything?

On the page titled [QQmlApplicationEngine Class](https://doc.qt.io/qt-6/qqmlapplicationengine.html),
I found the following information:

> Unlike QQuickView, QQmlApplicationEngine does not automatically create a root 
  window. If you are using visual items from Qt Quick, you will need to place 
  them inside of a [Window](https://doc.qt.io/qt-6/qml-qtquick-window.html).

Looking inside `ratatoskr/shareplugin/Main.qml`, no `Window` element is defined
(a situation inherited from the original `ubtd-20.04-master` code).

This is most likely the root cause.
