# vim:ft=zsh ts=2 sw=2 sts=2
#
# agnoster's Theme - https://gist.github.com/3712874
# A Powerline-inspired theme for ZSH
#
# # README
#
# In order for this theme to render correctly, you will need a
# [Powerline-patched font](https://github.com/Lokaltog/powerline-fonts).
# Make sure you have a recent version: the code points that Powerline
# uses changed in 2012, and older versions will display incorrectly,
# in confusing ways.
#
# In addition, I recommend the
# [Solarized theme](https://github.com/altercation/solarized/) and, if you're
# using it on Mac OS X, [iTerm 2](https://iterm2.com/) over Terminal.app -
# it has significantly better color fidelity.
#
# If using with "light" variant of the Solarized color schema, set
# SOLARIZED_THEME variable to "light". If you don't specify, we'll assume
# you're using the "dark" variant.
#
# # Goals
#
# The aim of this theme is to only show you *relevant* information. Like most
# prompts, it will only show git information when in a git working directory.
# However, it goes a step further: everything from the current user and
# hostname to whether the last call exited with an error to whether background
# jobs are running in this shell will all be displayed automatically when
# appropriate.

### Segment drawing
# A few utility functions to make it easy and re-usable to draw segmented prompts

CURRENT_BG='NONE'
BUILDPROMPT_CACHE_DIR=$HOME'/.cache/zsh_prompt/'
SRC_PATH=$PROMPT_THEME_ROOT'/src/main.cpp'

# EXPORTS
# these probably only needs to be exported once

export_init() {
  export _USERNAME="$USERNAME"
  export _DEFAULT_USER="$DEFAULT_USER"
  export _UID="$UID"
}


export_update() {
  export _RETVAL=$?
  echo $_RETVAL
}
purge_promptgen_cache() {
  rm -rf $BUILDPROMPT_CACHE_DIR
  mkdir -p $BUILDPROMPT_CACHE_DIR
}

recompile_promptgen() {
  g++ $SRC_PATH -std=c++23 -o $BUILDPROMPT_BIN -Wno-ignored-attributes -O3
}

recompile_if_needed() {
  mkdir -p $BUILDPROMPT_CACHE_DIR
  #TODO: checksum entire dir(if there is a usecase lol)
  SOURCE_CHECKSUM=$(sha256sum $SRC_PATH)
  #echo $SOURCE_CHECKSUM
  BUILDPROMPT_BIN="$BUILDPROMPT_CACHE_DIR""${SOURCE_CHECKSUM:0:16}"
  if [ ! -f $BUILDPROMPT_BIN ]; then
    recompile_promptgen
  fi
}

init_theme() {
  recompile_if_needed
  export_init
}

## Main prompt
build_prompt() {
  export _RETVAL=$?
  echo $("$BUILDPROMPT_BIN")
}


init_theme
PROMPT='%{%f%b%k%}$(build_prompt) '
