# musicxml2hum
MusicXML to Humdrum converter

## Compiling ##

To compile the command-line version of musicxml2hum2, type:

```bash
	make
```

This will create the executable `./bin/musicxml2hum`.  You will need `wget`
installed if compiling on an OS X computer.  This will download the two
external libraries: [humlib](https://github.com/craigsapp/humlib) 
and [pugixml](https://github.com/zeux/pugixml).

The executable is called `musicxml2hum` since it will be a replacement for
the current program `xml2hum` in [humextra](https://github.com/craigsapp/humextra).

`musicxml2hum` is currently under development.  Eventually this code will be incorporated into [humlib](https://github.com/craigsapp/humlib).


