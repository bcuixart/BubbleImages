# Bubble Images [Provisional Name]
This will eventually be a small game. A solid-shaded bubble appears. If clicked, it splits into 4 quarter-bubbles which, if clicked, also split into 4 sub-quarter-bubbles... gradually forming an image as the pixels become clearer when the bubbles are split.

This is clearly inspired by a website found in "The useless website", which I have not been able to find.

The main difference with that website is that here, users will be able to upload their own pictures, so it's not always the same image.

As such, before making the actual game, I need to work on parsing multiple photo formats (PNG, JPEG, PPM, WEBP...). Currently, only PPM is done.

I am making this in C to challenge myself, mostly. C++ would have been easier to use, especially for the image parser. But I guess I just hate myself.

I will use Raylib for the actual game, sooner or later.