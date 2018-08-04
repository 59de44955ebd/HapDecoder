DScmd v0.6 - DirectShow command-line player
(c) 2018 Valentin Schmidt

Usage:
======

DScmd <input file/URL>
DScmd <options>


Options:
========

-h                               Show this help
-i                               Activate interactive mode (keyboard control, see key list below)
-v <int verbosity>               Specify verbosity: 0=quiet, 1=errors, 2=warnings, 3=infos (=default)
-l                               List all registered DirectShow filters (CLSID and name)
-lc <category-CLSID>             List registered DirectShow filters of specified filter category
-file <input-file/URL>           Load a media/GRF file or media URL, render with default graph
-lav <input-file/URL>            Renders a media file or URL with LAV filters (either registered or provided in folder "filters")
-graph <graph-string>            Creates a DirectShow filter graph (see details below)
-noWindow                        Don't create any window (e.g. when playing only audio files)
-noClock                         Deactivate clock (render as fast as possible
-progress                        Print progress status updates (once per second)
-fullScreen                      Run in full screen mode
-alwaysOnTop                     Keep window always on top
-drop                            Activate support for dropping files into window (window mode only)
-hideCursor                      Hide mouse cursor (window mode only)
-cc <cc/subs-file>               Load CC/subtitles file into current graph
-loop <int n>                    Loop <n>-times, -1 for endless looping
-pos <int left,top>              Specify top left corner of video window
-size <int width,height>         Specify width and height of video window
-winCaption <string>             Specify custom caption (title) of video window (default="DScmd")
-winStyle <int style>            Specify custom style of video window (default=WS_TILEDWINDOW)
-sr <int left,top,width,height>  Specify video's source rect
-dr <int left,top,width,height>  Specify video's destination rect
-rate <float rate>               Specify playback rate (default = 1.0)
-startTime <int milliseconds>    Specify the start time in media file
-stopTime <int milliseconds>     Specify the stop time in media file
-volume <int volume>             Specify the audio volume (-10000=silent, 0=maximum)
-balance <int balance>           Specify the audio balance (-10000 to 10000)
-register                        Register rendered graph in system, so other DirectShow apps can read it
-saveGrf <filename>              Save rendered graph as GRF file
-elg <filename>                  Load Elecard Graph Viewer dynamically from ElGViewer.dll (32-bit only)
-osc <int port>                  Start OSC interface on local port <port>


Graph-String:
=============

A graph string has the form "<filter-list>!<connect-list>!<render-list>", where <connect-list> and <render-list>
are optional. <connect-list> can also be empty, i.e. you can use <filter-list>!!<render-list>.

Example (note: actually a single line, "^" only works in a batch script):

-graph {B98D13E7-55DB-4385-A33D-09FD1BA26338};src=big_buck_bunny.mp4;file=filters\LAVSplitter.ax,^
{EE30215D-164F-4A92-A4EB-9D4C13390F9F};file=filters\LAVVideo.ax,^
{51B4ABF3-748F-4E3B-A276-C828330E926A},^
{E8E73B6B-4CB3-44A4-BE99-4F7BCB96E491};file=filters\LAVAudio.ax!^
0:1,1:2,0:3!^
3

The above example string does the following:
It creates a graph based on the filters LAV Splitter Source, LAV Video Decoder, LAV Audio Decoder and VMR-9.
The 3 LAV filters are dynamically loaded from provided binary AX files.
Splitter, video decoder and VMR-9 are connected explicitely, whereas the audio decoder is rendered by the system.


Filter-list:
============

A list of filter items, separated by comma.
Each item starts with a CLSID, optionally followed by option strings, separated by semicolon.

Available options are:
dialog                           Show the filter's config dialog, if available
file=<file>                      Allows to dynamically load non-registered filter directly from AX/DLL file
src=<file/URL>                   Media file or URL, for source filters only
dest=<file>                      Destination file, for sink filters only
name=<codec-name>                Allows to specify a VCM/ACM-Codec by name. The CLSID in this case must be either
                                 CLSID_VideoCompressorCategory ({33D9A760-90C8-11D0-BD43-00A0C911CE86}) or
                                 CLSID_AudioCompressorCategory ({33D9A761-90C8-11D0-BD43-00A0C911CE86}).


Connect-list:
=============

A list of integer pairs, separated by comma.
Each pair has the format <int a>:<int b>, which means that the 2 filters with index a and b should be connected.
The indexes refer to the positions in the list that was passed as <filter-list>.


Render-list:
============

A list of integers, separated by comma, which specify the indexes of filters the DirectShow system should try to
render automatically. The indexes refer to the positions in the list that was passed as <filter-list>.


Keyboard control:
=================

Note: in window mode (default) keystrokes are - in addition to the CMD shell - also accepted by the window.

+/-                              Double/half playback rate (if supported by loaded graph)
0-9                              Show config dialog for <n>th filter in graph (if supported by filter)
LEFT/RIGHT                       Seek one frame backwards/forward (if supported by loaded graph)
PAGEUP/PAGEDOWN                  Seek 30 frames backwards/forward (if supported by loaded graph)
UP/DOWN                          Volume up/down (10% step)
c                                Toggle hide/show cursor (window mode only)
d                                Toggle CC/subs visibility (if a CC/subs file is loaded)
f                                Toggle full screen (if a video is loaded)
g                                Show graph using Elecard Graph Viewer if available, otherwise print filter list
h                                Print help
i                                Show video infos
l                                Toogle endless loop mode
m                                Mute (set volume to -10000)
o                                Load media/GRF file via open file dialog
p, SPACE                         Toggle pause/play
q, ESC                           Quit
r                                Rewind
t                                Toggle stay-on-top (window mode only)
u                                Load CC/subs file via open file dialog (requires loaded DirectVobSub filter)
y/x                              Step audio balance to left/right channel (5% step, if supported by graph)


OSC control:
============

/file <input-file/URL>           Load a media/GRF file or media URL, render with default graph
/frame <int n>                   Go to frame <n>
/fullscreen <int flag>           Activate/deactivate full screen mode with 1/0
/graph <string graph>            Load a list of filters, specified by CLSIDs (see details above)
/loop <int n>                    Set loop count to <n>, -1 for endless looping
/pause                           Pause
/play                            Play
/pos <int left,top>              Set top left corner of video window
/quit                            Quit
/rect <int l,t,r,b>              Set rect of video window
/rewind                          Rewind
/size <int width,height>         Set width and height of video window
/step <int n>                    Step by <n> frames (negative <n> to step backwards)
/time <int n>                    Go to millisecond <n>

