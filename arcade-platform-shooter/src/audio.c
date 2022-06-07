#include "shared.h"

void audio_sound_load(Mix_Chunk **chunk, const char *path) {
	*chunk = Mix_LoadWAV(path);
	if (!*chunk) {
		printf("Failed to load WAV: %s\n", Mix_GetError());
		exit(EXIT_FAILURE);
	}
}

void audio_music_load(Mix_Music **music, const char *path) {
	*music = Mix_LoadMUS(path);
	if (!*music) {
		printf("Failed to load music file %s: %s\n", path, Mix_GetError());
		exit(EXIT_FAILURE);
	}
}

void audio_sound_play(Mix_Chunk *sound) {
	Mix_PlayChannel(-1, sound, 0);
}

void audio_music_play(Mix_Music *music) {
	Mix_PlayMusic(music, -1);
}

void audio_setup() {
	SDL_Init(SDL_INIT_AUDIO);

	int audio_rate = 44100;
	u16 audio_format = MIX_DEFAULT_FORMAT;
	int audio_channels = 2;
	int audio_buffers = 4096;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		printf("SDL_mixer error: OpenAudio: %s\n", Mix_GetError());
		exit(1);
	}
}

