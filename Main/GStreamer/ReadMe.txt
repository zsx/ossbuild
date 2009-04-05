

Getting arbitrary text overlaid on top of video:


<YoYo> does playbin/playbin2 support text overlay?

<MikeS__> Yes/

<YoYo> i'm looking at the documentation - i see it supports subtitles

<YoYo> but i don't see a property for setting the text

<MikeS__> oh, separate text overlay? playbin/playbin2 don't provide API for that directly.

<MikeS__> However, you can set a video sink on them - and it could be a bin that has a textoverlay element in it, that you control

<YoYo> gst-launch playbin2 uri=/video.avi ! textoverlay text=hi there ! autovideosink   or something of that nature?

<MikeS__> well, you can't do it from gst-launch, but you'd set the 'video-sink' property on a playbin2 instance to a bin that contains textoverlay and autovideosink

<MikeS__> pretty easy

<YoYo> great - ty so much
