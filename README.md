# Bubble Images [Provisional Name]
This is a small game where a solid-shaded bubble appears. If clicked, it splits into 4 quarter-bubbles which, if clicked, also split into 4 sub-quarter-bubbles... gradually forming an image as the pixels become clearer when the bubbles are split.

This is clearly inspired by the website koalastothemax.

The main difference with that website is that here, users will be able to upload their own pictures, so it's not always the same image.

As such, before making the actual game, I need to work on parsing multiple photo formats (PNG, JPEG, PPM, WEBP...). Currently, only PPM is fully done; and PNG is functional, but needs improvements.

I am making this in C to challenge myself, mostly. C++ would have been easier to use, especially for the image parser. But I guess I just hate myself.

I am using Raylib for the actual game.