# How to use

- make

- Default camera feed (/dev/video0): `./bin/tifo`
- Any feed (webcam, video file, rtsp stream) : `./bin/tifo <feed>`

# Shortcuts

## Edges (Canny)

- **E** display raw detected edges
- **B** apply border darkening
- **D** apply border dilation/thickening
- **RIGHT** and **LEFT** arrows to select blur function
- **L** / **H** + **UP** / **DOWN** to update low/high Canny thresholds

## Color

- **P** compute color palette (Color Quantization)
- **C** apply color quantization
- **S** apply color saturation boost
- **X**  color contrast correction
- **UP** / **DOWN** arrows to update saturation value

## Misc
- **N** apply pixel filter

## Utils
- **SPACE** to freeze the video stream on the current frame
- **TAB** display shortcuts
