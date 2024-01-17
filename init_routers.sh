#!/bin/bash
for id in {1..4}
do
  gnome-terminal -- ./router $id
done
