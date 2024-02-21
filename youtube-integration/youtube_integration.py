import threading
import traceback
from typing import Optional
import time
from queue import Queue, Empty

from pytchat.core import PytchatCore
from rcon.source import Client
from rcon.exceptions import WrongPassword, EmptyResponse

# noinspection PyUnresolvedReferences
import obspython as obs

from pytchat_fix import (
    Processor,
    Message,
    extract_video_id,
    is_chat_enabled,
)

STREAM_ID = None
SOURCE_NAME = ""
RCON_HOST = "127.0.0.1"
RCON_PORT = "27015"
RCON_PASSWORD = ""

voteNumber = -1
votes = {}
voteEffectsNumber = []
voteKeywords = []


def update_game_votes(newVote, oldVote = None):
    global voteNumber, voteEffectsNumber, votes
    vote_params = [str(newVote)]
    if oldVote is not None:
        vote_params.append(str(oldVote))
    print(rcon.run("chaos_vote_internal_update", str(voteNumber), *vote_params, enforce_id=False))


def poll_game():
    global voteNumber, voteEffectsNumber, votes
    raw_resp = rcon.run("chaos_vote_internal_poll", enforce_id=False)
    response = raw_resp.split('rcon from "', 1)[0].strip()
    if response == "":
        return False  # the game is not quite ready yet.

    vote_number, *effects = response.split(";")
    vote_number = int(vote_number)

    voteEffectsNumber = effects
    if vote_number != voteNumber:
        voteNumber = vote_number
        votes = {}
    return True


def game_loop():
    global rcon
    STARTUP.wait()
    already_printed_err = False
    faulty_password = ""
    rcon = None
    while True:
        if SHUTDOWN:
            return

        time.sleep(1)
        if faulty_password == RCON_PASSWORD:
            continue  # wait for the user to change password
        else:
            faulty_password = ""
        try:
            if rcon is None:
                rcon = Client(RCON_HOST, int(RCON_PORT), passwd=RCON_PASSWORD)
                rcon.connect(True)
            poll_game()
            if already_printed_err:
                print("poll resumed as normal.")
            already_printed_err = False
        except ConnectionError as e:
            if not already_printed_err:
                print("poll failed", e)
                already_printed_err = True
            rcon = None
        except WrongPassword:
            print("rcon wrong password")
            faulty_password = RCON_PASSWORD
            rcon = None
        except EmptyResponse:
            pass
        except Exception as e:
            if not already_printed_err:
                print(
                    "poll unexpected exception:", eYT
                )  # i broke something. log and bail
                already_printed_err = True
            rcon = None



STARTUP = threading.Event()
CAN_CONNECT = threading.Event()
CAN_CONNECT.set()
SHUTDOWN = False
chat: Optional[PytchatCore] = None
rcon: Client = None
game_thread: Optional[threading.Thread] = None
chat_thread: Optional[threading.Thread] = None


def channel_loop():
    global STREAM_ID, chat, chat_thread
    try:
        while True:
            new_stream_id = channel_queue.get(timeout=2)
            channel_queue.task_done()
    except Empty:
        if chat:
            chat.terminate()
            chat_thread.join()
            chat = None
        STREAM_ID = new_stream_id
        CAN_CONNECT.clear()
        chat_thread = threading.Thread(None, chat_loop, daemon=False)
        chat_thread.start()


channel_queue = Queue()
channel_thread: Optional[threading.Thread] = None


def chat_loop():
    global chat
    STARTUP.wait()
    try:
        if not STREAM_ID:
            print(
                "The provided URL seems to be incorrect or invalid. Please recheck the stream URL and try again."
            )
            chat = None
            CAN_CONNECT.set()
            return
        status = is_chat_enabled(STREAM_ID)
        if status is False:
            print(
                "The provided URL doesn't seem to be a live stream, or chat for this stream is disabled. Please make sure you've entered a valid stream URL with an enabled chat."
            )
            chat = None
            CAN_CONNECT.set()
            return
        elif status is None:
            print(
                "The provided URL seems to be incorrect or invalid. Please recheck the stream URL and try again."
            )
            chat = None
            CAN_CONNECT.set()
            return
        chat = PytchatCore(STREAM_ID, processor=Processor(), interruptable=False)
        while chat.is_alive():
            if SHUTDOWN:
                return

            chat_data = chat.get()
            if not CAN_CONNECT.is_set():
                print("connected")
                CAN_CONNECT.set()
            for message in chat_data.items:
                message: Message = message
                text = message.message.strip().upper()
                print(f"{message.author.name} said: {message.message}")
                uppercase_keywords = [kw.upper() for kw in voteKeywords]
                if text in uppercase_keywords:
                    kwIndex = uppercase_keywords.index(text)
                    oldVote = None
                    if message.author.channelId in votes:
                        oldVote = votes[message.author.channelId]
                    #check range of index because we don't want a 2 when the range in OBS is 4-7
                    if voteNumber % 2 == 0:
                        if kwIndex >= 4:
                            votes[message.author.channelId] = kwIndex - 4
                    else:
                        if kwIndex <= 3:
                            votes[message.author.channelId] = kwIndex
                    if message.author.channelId in votes:
                        update_game_votes(votes[message.author.channelId], oldVote)
            time.sleep(1.5)

    except Exception as e:
        print(
            "Error in chat happened. ",
            e,
            "\nTo start chat again press button 'Reconnect to youtube'.",
        )
        chat.terminate()
        chat = None
        CAN_CONNECT.set()
    CAN_CONNECT.set()


def set_text(source: str, text: str):
    s = obs.obs_get_source_by_name(source)
    if s:
        settings = obs.obs_data_create()
        obs.obs_data_set_string(settings, "text", text)
        obs.obs_source_update(s, settings)
        obs.obs_data_release(settings)
        obs.obs_source_release(s)


def update_source():
    if SOURCE_NAME == "":
        return
    output = f"Vote #{voteNumber}\n"
    offset = 0
    for i, voteEffect in enumerate(voteEffectsNumber):
        if i == 0 and voteNumber % 2 == 0:
            offset = 4
        name, amount = voteEffect.split(":")
        keyword = "?" if i > len(voteKeywords) - 1 else voteKeywords[i + offset]
        output = output + f"{keyword} {name}: {amount}\n"
        if i == 3 and voteNumber % 2 == 1:
            break
    set_text(SOURCE_NAME, output[:-1])  # exclude final newline


def script_properties():
    props = obs.obs_properties_create()
    obs.obs_properties_add_text(props, "stream_url", "Stream URL", obs.OBS_TEXT_DEFAULT)

    p = obs.obs_properties_add_list(
        props,
        "source",
        "Text Source",
        obs.OBS_COMBO_TYPE_EDITABLE,
        obs.OBS_COMBO_FORMAT_STRING,
    )
    sources = obs.obs_enum_sources()
    if sources:
        for source in sources:
            source_id = obs.obs_source_get_unversioned_id(source)
            if source_id == "text_gdiplus" or source_id == "text_ft2_source":
                name = obs.obs_source_get_name(source)
                obs.obs_property_list_add_string(p, name, name)
    obs.source_list_release(sources)

    obs.obs_properties_add_text(props, "rcon_host", "RCON host", obs.OBS_TEXT_DEFAULT)
    obs.obs_properties_add_text(
        props, "rcon_password", "RCON password", obs.OBS_TEXT_PASSWORD
    )
    obs.obs_properties_add_editable_list(
        props,
        "vote_keywords",
        "Vote keywords",
        obs.OBS_EDITABLE_LIST_TYPE_STRINGS,
        None,
        None,
    )

    def reconnect(*args, **kwargs):
        global chat_thread
        print("reconnecting to youtube")
        chat.terminate()
        chat_thread.join()
        CAN_CONNECT.clear()
        chat_thread = threading.Thread(None, chat_loop, daemon=False)
        chat_thread.start()

    obs.obs_properties_add_button(
        props, "reconnect_button", "Reconnect to youtube", reconnect
    )
    return props


def script_defaults(settings):
    obs.obs_data_set_default_string(
        settings, "stream_url", "https://www.youtube.com/watch?v="
    )
    obs.obs_data_set_default_string(settings, "rcon_host", "127.0.0.1:27015")

    obs_array = obs.obs_data_array_create()
    for i in ["1", "2", "3", "4", "5", "6", "7", "8"]:
        item = obs.obs_data_create()
        obs.obs_data_set_string(item, "value", i)
        obs.obs_data_array_push_back(obs_array, item)
        # obs.obs_data_release(item)
    obs.obs_data_set_default_array(settings, "vote_keywords", obs_array)
    # obs.obs_data_array_release(obs_array)


def script_update(settings):
    # i feel like i'm doing something wrong.
    global voteKeywords, STREAM_ID, SOURCE_NAME, RCON_HOST, RCON_PORT, RCON_PASSWORD, channel_thread
    new_stream_id = extract_video_id(obs.obs_data_get_string(settings, "stream_url"))
    if not STARTUP.is_set():
        STREAM_ID = new_stream_id
    if STREAM_ID != new_stream_id:
        CAN_CONNECT.wait(2.0)  # Prevents updates while the chat is starting
        if not channel_thread or not channel_thread.is_alive():
            channel_thread = threading.Thread(None, channel_loop)
            channel_thread.start()
        channel_queue.put(new_stream_id)
        # STREAM_ID = stream_id
    SOURCE_NAME = obs.obs_data_get_string(settings, "source")
    # TODO: Verify port here. We may have an error if we don't
    RCON_HOST, RCON_PORT = obs.obs_data_get_string(settings, "rcon_host").split(":", 1)
    RCON_PASSWORD = obs.obs_data_get_string(settings, "rcon_password")

    voteKeywords = []
    obs_votes_keywords = obs.obs_data_get_array(settings, "vote_keywords")
    for i in range(obs.obs_data_array_count(obs_votes_keywords)):
        item = obs.obs_data_array_item(obs_votes_keywords, i)
        value = obs.obs_data_get_string(item, "value")
        voteKeywords.append(value)
        obs.obs_data_release(item)
    obs.obs_data_array_release(obs_votes_keywords)
    print("data updated")
    STARTUP.set()


def script_load(settings):
    global SHUTDOWN, STARTUP, game_thread, chat_thread
    SHUTDOWN = False
    STARTUP.clear()
    obs.timer_add(update_source, 1000)
    game_thread = threading.Thread(None, game_loop, daemon=False)
    game_thread.start()
    CAN_CONNECT.clear()
    chat_thread = threading.Thread(None, chat_loop, daemon=False)
    chat_thread.start()


def script_unload():
    global game_thread, chat_thread, SHUTDOWN
    obs.timer_remove(update_source)
    SHUTDOWN = True
    for thread in (game_thread, chat_thread):
        if thread is not None and thread.is_alive():
            # Wait for 5 seconds, if it doesn't exit just move on not to block
            # OBS main thread. Logging something about the failure to properly exit
            # is advised.
            thread.join(5.0)
    game_thread = None
    chat_thread = None


def script_description():
    return """Youtube chat voting plugin for HL2Chaos mod
    
Made by holy-jesus, based on code written by acuifex
Released under AGPLv3 license"""
