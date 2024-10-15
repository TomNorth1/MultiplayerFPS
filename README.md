# Unreal Engine Multiplayer Game

## A Multiplayer First Person shooter made in Unreal Engine 4.25 using C++

DISCLAIMER: This repo only features the source files as the full project contains files above 100MB

This project is a fully networked first person shooter made using C++ which features:
* Replicated inventory system
* Players can loot the bodies of dead players
* Pattern based recoil patterns similar to games like Valorant, Rust and CSGO
* Replicated locked doors and keys
* Damage system based on what part of players body is hit
* Fully networked player animations
* Mansion map inspired by early Resident Evil games

The project was a passion project and learning experience for me as I wanted to explore the unique challenges behind developing multiplayer games and the different considerations that need to be made when implementing new features in a networked scenario.

## Examples

![](https://i.ibb.co/ZY3k9RQ/Screenshot-2024-10-15-174950.png)

Fully replicated inventory system and item system featuring health items, ammo, weapons, and key items.

![](https://i.ibb.co/w4V3n6C/Screenshot-2024-10-15-175420.png)

Able to loot other inventories i.e dead players. Inventory system is modular so can be added to any Actor in game so makes it easy to add something like storage lockers.

![](https://i.giphy.com/media/v1.Y2lkPTc5MGI3NjExbnd3cjh4Zm1vcXlwNW4yeGxtMmcwMWx4ZzczNHUyZmp4eDRtajhnYSZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/5ttUTaCpOX5LI4qcz5/giphy-downsized-large.gif)

Demonstration of Valorant esque recoil system. Bullets go up in a vertical pattern till they meet a max height then start to have more horizontal recoil. Patterns can be changed on a per weapon basis and can adjust the randomness of each pattern.

Further examples of this project can be found in my YouTube portfolio video here:
↓

[![](https://img.youtube.com/vi/g8mHCsSSICo/0.jpg)](https://www.youtube.com/watch?v=g8mHCsSSICo)
