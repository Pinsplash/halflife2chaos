import re

from pytchat.processors.default.processor import DefaultProcessor
from pytchat.processors.default.renderer.textmessage import LiveChatTextMessageRenderer
import pytchat.core
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
pytchat.core.pytchat.headers = HEADERS
pytchat.config.headers = HEADERS
pytchat.config.m_headers = HEADERS
pytchat.config.m_headers["User-Agent"] = "Mozilla/5.0 (Linux; Android 10; K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36"


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
    timestamp: int
    message: str
    author: Author
    type = "textMessage"


class Processor(DefaultProcessor):
    def __init__(self):
        self.first = True
        self.abs_diff = 0
        self.renderers = {"liveChatTextMessageRenderer": LiveChatTextMessageRenderer()}


PATTERN_YTURL = re.compile(
    r"((?<=(v|V)/)|(?<=be/)|(?<=/live/)|(?<=/video/)|(?<=(\?|\&)v=)|(?<=embed/))([\w-]+)"
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


def is_chat_enabled(stream_id: str):
    with httpx.Client(http2=True, headers=HEADERS) as client:
        response = client.get(f"https://www.youtube.com/live_chat?is_popout=1&v={stream_id}", headers=HEADERS)
        if "invalidationContinuationData" in response.text:
            return True
        elif response.status_code == 200:
            return False
        else:
            return None
