#!/bin/sh

cd ~/goinfre
git clone --depth 1 https://github.com/Homebrew/brew.git
cd brew
bin/brew install siege
