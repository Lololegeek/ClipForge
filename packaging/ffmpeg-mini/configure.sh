export PATH=/mingw64/bin:/usr/bin
export PKG_CONFIG_PATH=/mingw64/lib/pkgconfig
cd "/d/Code 2/LoloClips/.deps/ffmpeg-build/ffmpeg-7.1.1" || exit 1
./configure --prefix="/d/Code 2/LoloClips/.deps/ffmpeg-build/out" --target-os=mingw32 --arch=x86_64 \
 --enable-gpl --enable-version3 --enable-shared --disable-static \
 --disable-doc --disable-debug --disable-network --disable-autodetect --disable-ffplay --disable-avdevice \
 --disable-encoders --disable-filters --disable-hwaccels --disable-indevs --disable-outdevs --disable-protocols \
 --enable-protocol=file,pipe,concat,subfile \
 --enable-zlib --enable-libx264 --enable-libx265 --enable-libwebp \
 --enable-encoder=wrapped_avframe,rawvideo,pcm_s16le,pcm_u8,libx264,libx265,libwebp_anim,libwebp,aac,gif,png,mjpeg,pcm_s16le,pcm_f32le,pcm_s16be \
 --enable-filter=scale,overlay,split,asplit,palettegen,paletteuse,loudnorm,aresample,aformat,format,anull,null,trim,atrim,setpts,asetpts,amix,volume,fps,crop,pad,transpose,hflip,vflip,movie,color,copy,acopy,anullsrc,nullsink,anullsink,buffer,buffersink,abuffer,abuffersink,setsar,setdar,concat,adelay,afade,fade,format,pan,amerge,atempo,areverse,reverse,volumedetect,showinfo,thumbnail,select,aselect \
 --extra-cflags="-O2" --extra-ldflags="-static-libgcc" --pkg-config-flags="--static"
echo CONFIGURED $?
