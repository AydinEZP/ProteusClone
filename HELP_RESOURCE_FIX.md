# Help Resource Fix

This version confirms that `resources/help/user_guide.html` is included through `resources/resources.qrc` and compiled by CMake.

## Important Qt resource path

The QRC entry uses an alias:

```xml
<qresource prefix="/help">
    <file alias="user_guide.html">help/user_guide.html</file>
</qresource>
```

Therefore the runtime path used by `HelpDialog` is valid:

```cpp
QFile file(":/help/user_guide.html");
```

CMake also includes the resource file:

```cmake
list(APPEND SOURCES resources/resources.qrc)
```
