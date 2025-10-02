#ifndef AVFORMAT_ISOM_H
#define AVFORMAT_ISOM_H


int ff_mov_iso639_to_lang(const char lang[4], int mp4);
int ff_mov_lang_to_iso639(unsigned code, char to[4]);


#endif

