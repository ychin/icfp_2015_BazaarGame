HELLO JUDGES!

Our solution is on Windows and requires (hopefully any version of) Microsoft
Visual Studio to build.  We are using Visual Studio 2010.

We have included a prebuilt binary of play_icfp2015.exe for your convenience.

########################
## BUILD INSTRUCTIONS ##
########################

1) Start a Visual Studio command prompt (usually in All Programs > Visual
Studio whatever > Visual Studio Tools > Visual Studio Command Prompt). The
batch file supports command prompts of Visual Studio 2010, 2012, and 2013.
Visual Studio 2015 should work too but is untested.

2) cd to the directory with this README file

3) Run make.bat

4) The executable will be called play_icfp2015.exe in this directory!

####################
## SOLUTION NOTES ##
####################

We created several AIs that greedily choose moves according to a variety
of criteria.  We then use a genetic algorithm to try to find the best plan of
which AI to use for each move.

We spend more time working on the current best plan but keep up to 20
plans available which can be worked on, to attempt to avoid local maxima.

We assume the judges will pass all appropriate "-p" parameters to our
program; during development we found several power phrases, but the
specification seems to say the judging process will inform our code
of all known power phrases.  (And that there are no more than 18)

We ignore the "-m" parameter, our solution pre-allocates almost all
data structures.

Without the "-t" parameter our solution will run for 24 hours.  We hope
the judges will specify this parameter for all programs.

We currently ignore the "-c" parameter and run single threaded.

#################
## EXTRA STUFF ##
#################

Game.exe is our testbed tool, built with the Temple Gates XEngine.
Code for this tool is in GameApp.cpp, but the project won't build
without the engine (sorry!)

Keys are defined in GameInputHandle(), but here's a summary:

SPACE: Reset the game (and advance seeds if the problem has multiple seeds)

Q/E: Rotate
A/D: Move sideways
Z/C: Move downwards

The game will not let you play invalid 'stuttering moves'.  This is
somewhat unintuitive for human players :)

1: Play a random move
2: Let the current AI choose a move.
n/m: Choose the current AI

5: Reset the genetic AI for the current problem, seed 0
6: Run one generation of the genetic AI

There are other key-commands defined, but they may summon the Old Ones.
Tread carefully.
