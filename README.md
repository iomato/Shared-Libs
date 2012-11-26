## About

Repository containing C++ third party libraries which are dependencies of others projects, specially [Omni](/midiway/Omni). Some will be the original copies, but there is also some customized ones, so plz dont redistribute them.

When possible/available, I prefer to add them as a Git submodule. Refer [here](http://dropshado.ws/post/20058825150/git-submodules) for some commands you might need to use. Basically after cloning you need to

```ruby
git submodule init
git submodule update
```

This copies make it easy to you compile projects which depends on them, without the need to download each of them separately. And in order to let you not need to compile each library source, note that I also put here some of them already compiled, but I'm not sure if you will be able to use these .lib files due the likely differences of compilers, environment, etc.



***

Libraries available:

- TinyXML: from developer source - as a submodule

- Sciter 2: from [here](http://www.terrainformatica.com/)

- Scintilla: only the sources customized for Omni, download the rest (ensure the correct version) from [here](http://www.scintilla.org/)

Libraries that are actually needed for Omni, but are missing (just download them)

- StormLib: from [here](http://www.zezula.net/en/mpq/download.html)

- CrashRpt (the SDK comes already compiled): from [here](http://code.google.com/p/crashrpt/)

Libraries you have to compile before usage by dependent projects:

- StormLib

- Scintilla
