import asyncio
import re
import traceback

from pytchat import LiveChatAsync, util, config, exceptions
from pytchat.processors.default.processor import DefaultProcessor
from pytchat.processors.default.renderer.textmessage import LiveChatTextMessageRenderer
from pytchat.paramgen import liveparam
import pytchat.core_async.livechat
import pytchat.config

import httpx


HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36 Edg/119.0.0.0",
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8",
    "Accept-Language": "en-US,en;q=0.5",
    "DNT": "1",
    "Connection": "keep-alive",
    "Upgrade-Insecure-Requests": "1",
    "Sec-Fetch-Dest": "document",
    "Sec-Fetch-Mode": "navigate",
    "Sec-Fetch-Site": "none",
    "Sec-Fetch-User": "?1",
    "Pragma": "no-cache",
    "Cache-Control": "no-cache",
}
pytchat.core_async.livechat.headers = HEADERS
pytchat.config.headers = HEADERS
pytchat.config.m_headers = HEADERS
pytchat.config.m_headers["user-agent"] = "Mozilla/5.0 (Linux; Android 10; K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36"


class Author:
    name: str
    channelId: str
    channelUrl: str
    imageUrl: str
    type: str
    isVerified: bool
    isChatOwner: bool
    isChatSponsor: bool
    isChatModerator: bool


class Message:
    id: str
    message: str
    author: Author
    type = "textMessage"


class Processor(DefaultProcessor):
    def __init__(self):
        self.first = True
        self.abs_diff = 0
        self.renderers = {"liveChatTextMessageRenderer": LiveChatTextMessageRenderer()}


class LiveChatAsync(LiveChatAsync):
    def _setup(self):
        self.listen_task = loop.create_task(self._fetch_continuation())

    async def _fetch_continuation(self):
        """Fetch continuation parameter"""
        if not self.continuation:
            channel_id = await util.get_channelid_async(self._client, self._video_id)
            self.continuation = liveparam.getparam(
                self._video_id, channel_id, past_sec=3
            )

    async def _get_chat_component(self):
        try:
            async with httpx.AsyncClient(http2=True) as client:
                if self.continuation and self._is_alive:
                    contents = await self._get_contents(
                        self.continuation, client, config.headers
                    )
                    metadata, chatdata = self._parser.parse(contents)
                    timeout = metadata["timeoutMs"] / 1000
                    chat_component = {
                        "video_id": self._video_id,
                        "timeout": timeout,
                        "chatdata": chatdata,
                    }
                    self.continuation = metadata.get("continuation")
                    self._last_offset_ms = metadata.get("last_offset_ms", 0)
                    return chat_component
        except exceptions.ChatParseException as e:
            self._logger.debug(f"[{self._video_id}]{str(e)}")
            # self._raise_exception(e)
        except Exception as e:
            self._logger.error(f"{traceback.format_exc(limit=-1)}")
            # self._raise_exception(e)

    async def get(self):
        if self.is_alive():
            chat_component = await self._get_chat_component()
            return self.processor.process([chat_component])
        else:
            return []

    def terminate(self):
        if not self.is_alive():
            return
        self._is_alive = False
        self.processor.finalize()


PATTERN_YTURL = re.compile(
    r"((?<=(v|V)/)|(?<=be/)|(?<=/live/)|(?<=(\?|\&)v=)|(?<=embed/))([\w-]+)"
)
YT_VIDEO_ID_LENGTH = 11


def extract_video_id(url_or_id: str) -> str:
    ret = ""
    if not isinstance(url_or_id, str):
        return None
    if "[" in url_or_id:
        url_or_id = url_or_id.replace("[", "").replace("]", "")

    if len(url_or_id) == YT_VIDEO_ID_LENGTH:
        return url_or_id
    match = re.search(PATTERN_YTURL, url_or_id)
    if match is None:
        return None
    try:
        ret = match.group(4)
    except IndexError:
        return None

    if ret is None or len(ret) != YT_VIDEO_ID_LENGTH:
        return None
    return ret


async def is_stream_live(stream_id: str):
    async with httpx.AsyncClient(http2=True, headers=HEADERS) as client:
        response = await client.get(f"https://www.youtube.com/live_chat?is_popout=1&v={stream_id}", headers=HEADERS)
        print(response.text)
        if "invalidationContinuationData" in response.text:
            return True
        elif response.status_code == 200:
            return False
        else:
            return None


if __name__ == "__main__":

    async def main():
        # chat = LiveChatAsync("https://www.youtube.com/watch?v=g9WTuSQgfUU", processor=Processor())
        while False:
            print(chat.continuation)
            chat_data = await chat.get()
            async for message in chat_data.async_items():
                message: Message = message
                print(message.author.channelId, message.message)
        print()
        

    loop = asyncio.new_event_loop()
    loop.run_until_complete(main())
