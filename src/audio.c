#include "audio.h"

static scm_t_bits sample_tag;
static scm_t_bits audio_stream_tag;

GmkSample*
check_sample (SCM sample_smob) {
    scm_assert_smob_type (sample_tag, sample_smob);

    return (GmkSample *) SCM_SMOB_DATA (sample_smob);
}

static SCM
load_sample (SCM s_file) {
    SCM smob;
    const char *file = scm_to_locale_string (s_file);
    GmkSample *sample = (GmkSample *) scm_gc_malloc (sizeof (GmkSample), "sample");

    sample->sample = NULL;

    SCM_NEWSMOB (smob, sample_tag, sample);

    sample->sample = al_load_sample (file);

    if (!sample->sample) {
        fprintf (stderr, "failed to load audio sample: %s\n", file);
    }

    return smob;
}

static SCM
play_sample (SCM sample_smob, SCM s_gain, SCM s_pan, SCM s_speed) {
    GmkSample *sample = check_sample (sample_smob);
    float gain = scm_to_double (s_gain);
    float pan = scm_to_double (s_pan);
    float speed = scm_to_double (s_speed);

    al_play_sample (sample->sample, gain, pan, speed, ALLEGRO_PLAYMODE_ONCE, NULL);

    return SCM_UNSPECIFIED;
}

static size_t
free_sample (SCM sample_smob) {
    GmkSample *sample = (GmkSample *) SCM_SMOB_DATA (sample_smob);

    al_destroy_sample (sample->sample);

    scm_gc_free (sample, sizeof (GmkSample), "sample");

    return 0;
}

static int
print_sample (SCM sample_smob, SCM port, scm_print_state *pstate) {
    scm_puts ("#<sample >", port);

    /* non-zero means success */
    return 1;
}

GmkAudioStream*
check_audio_stream (SCM audio_stream_smob) {
    scm_assert_smob_type (audio_stream_tag, audio_stream_smob);

    return (GmkAudioStream *) SCM_SMOB_DATA (audio_stream_smob);
}

static SCM
load_audio_stream (SCM s_file) {
    SCM smob;
    const char *file = scm_to_locale_string (s_file);
    GmkAudioStream *audio_stream = (GmkAudioStream *) scm_gc_malloc (sizeof (GmkAudioStream),
                                                               "audio_stream");

    audio_stream->stream = NULL;

    SCM_NEWSMOB (smob, audio_stream_tag, audio_stream);

    /* Some magic numbers here taken from the allegro 5 example for streams. */
    audio_stream->stream = al_load_audio_stream (file, 4, 2048);

    if (!audio_stream->stream) {
        fprintf (stderr, "failed to load audio audio_stream: %s\n", file);
    }

    return smob;
}

static SCM
play_audio_stream (SCM audio_stream_smob, SCM s_gain, SCM s_pan, SCM s_speed,
                   SCM s_loop) {
    GmkAudioStream *audio_stream = check_audio_stream (audio_stream_smob);
    float gain = scm_to_double (s_gain);
    float pan = scm_to_double (s_pan);
    float speed = scm_to_double (s_speed);
    bool loop = scm_to_bool (s_loop);
    ALLEGRO_PLAYMODE playmode = loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE;

    al_set_audio_stream_gain (audio_stream->stream, gain);
    al_set_audio_stream_pan (audio_stream->stream, pan);
    al_set_audio_stream_speed (audio_stream->stream, speed);
    al_set_audio_stream_playmode (audio_stream->stream, playmode);
    al_attach_audio_stream_to_mixer (audio_stream->stream, al_get_default_mixer ());

    scm_remember_upto_here_1 (audio_stream_smob);

    return SCM_UNSPECIFIED;
}

static size_t
free_audio_stream (SCM audio_stream_smob) {
    GmkAudioStream *audio_stream = (GmkAudioStream *) SCM_SMOB_DATA (audio_stream_smob);

    al_destroy_audio_stream (audio_stream->stream);

    scm_gc_free (audio_stream, sizeof (GmkAudioStream), "audio_stream");

    return 0;
}

static int
print_audio_stream (SCM audio_stream_smob, SCM port, scm_print_state *pstate) {
    scm_puts ("#<audio-stream >", port);

    /* non-zero means success */
    return 1;
}

void
gmk_init_sample (void) {
    sample_tag = scm_make_smob_type ("sample", sizeof (GmkSample));
    scm_set_smob_mark (sample_tag, 0);
    scm_set_smob_free (sample_tag, free_sample);
    scm_set_smob_print (sample_tag, print_sample);

    scm_c_define_gsubr ("load-sample", 1, 0, 0, load_sample);
    scm_c_define_gsubr ("play-sample", 4, 0, 0, play_sample);

    scm_c_export ("load-sample", NULL);
    scm_c_export ("play-sample", NULL);
}

void
gmk_init_audio_stream (void) {
    audio_stream_tag = scm_make_smob_type ("audio-stream", sizeof (GmkAudioStream));
    scm_set_smob_mark (audio_stream_tag, 0);
    scm_set_smob_free (audio_stream_tag, free_audio_stream);
    scm_set_smob_print (audio_stream_tag, print_audio_stream);

    scm_c_define_gsubr ("load-audio-stream", 1, 0, 0, load_audio_stream);
    scm_c_define_gsubr ("play-audio-stream", 5, 0, 0, play_audio_stream);

    scm_c_export ("load-audio-stream", NULL);
    scm_c_export ("play-audio-stream", NULL);
}
