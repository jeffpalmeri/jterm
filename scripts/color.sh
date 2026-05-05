#! /usr/bin/env bash

for a in 0 1 2 3 4 5 6 7
do
  COLOR=$((30 + a))
  printf "\033[${COLOR}mcolor\033[30m\n"
done

printf "\n"

for a in 0 1 2 3 4 5 6 7
do
  COLOR=$((90 + a))
  printf "\033[${COLOR}mcolor\033[30m\n"
done

printf "\n"

for a in 0 1 2 3 4 5 6 7
do
  COLOR=$((40 + a))
  printf "\033[${COLOR}mcolor\033[40m\n"
done

printf "\n"

for a in 0 1 2 3 4 5 6 7
do
  COLOR=$((100 + a))
  printf "\033[${COLOR}mcolor\033[40m\n"
done

printf "\n"
