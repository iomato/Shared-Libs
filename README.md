## About

Repository containing C++ third party libraries which are dependencies of others projects, specially [Omni](/midiway/Omni). Some will be the original copies, but there is also some customized ones, so plz dont redistribute them.

When possible/available, I prefer to add them as a Git submodule. So after cloning you might need to

```ruby
git submodule init
git submodule update
```

This copies make it easy to you compile projects which depends on them, whithout the need to download each of them separately. And in order to let you not need to compile each library source, note that I also put here some of them already compiled, but I'm not sure if you will be able to use these .lib files due the likely differences of compilers, environment, etc.



***

Libraries available:

- TinyXML: from developer source - submodule

- Sciter 2 (preview version): from [here](http://www.terrainformatica.com/)

Libraries I will eventually add (they are actually needed for Omni):

- ATLServer

- distorm3

- Scintilla (Omni customized)

- StormLib

- CrashRpt

Libraries you have to compile before usage by dependent projects:

- Scintilla (.dll)

- StormLib (.lib and .dll)
