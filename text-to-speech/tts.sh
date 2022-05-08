#!/bin/bash
# python3 -m pip install google-cloud-texttospeech
# python3 -m pip install google-api-python-client
export GOOGLE_APPLICATION_CREDENTIALS=PUT-YOUR-KEY-HERE.json
#prevdir=`pwd`
#source env/bin/activate
#cd $prevdir
python3 tts_worker.py $*
#deactivate
