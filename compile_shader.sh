#!/bin/zsh

glslangValidator -V -o workdir/imgui_frag.spv src/dearimgui/imgui_shader.frag
glslangValidator -V -o workdir/imgui_vert.spv src/dearimgui/imgui_shader.vert
glslangValidator -V -o workdir/bsp_frag.spv src/bsp/bsp_shader.frag
glslangValidator -V -o workdir/bsp_vert.spv src/bsp/bsp_shader.vert