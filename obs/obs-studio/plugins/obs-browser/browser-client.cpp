/******************************************************************************
 Copyright (C) 2014 by John R. Bradley <jrb@turrettech.com>
 Copyright (C) 2018 by Hugh Bailey ("Jim") <jim@obsproject.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "browser-client.hpp"
#include "obs-browser-source.hpp"
#include "base64/base64.hpp"
#include "json11/json11.hpp"
#include <obs-frontend-api.h>
#include <obs.hpp>
#include <util/platform.h>

using namespace json11;

// bililive[
#include <stdlib.h>
#include <list>
#include "include/cef_waitable_event.h"

extern bool QueueCEFTask(std::function<void()> task);
namespace
{
    // cef_proxy_dll::cef_proxy_shutdown_module
    CefRefPtr<CefWaitableEvent> g_can_safely_cef_shutdown;

    /*
     * 当浏览器源打开那种长时间都无法完成加载的网页时（比如墙外网）程序立刻退出，
     * 此时的CefBrowser::GetHost()->CloseBrowser(true)并不能立刻关闭并释放CEF对象CefBrowser，
     * 那么在CefShutdown时就会引发shutdown_checker::AssertNotShutdown()的!IsCefShutdown()断言，
     * Release下应该没什么大碍（没试过），Debug下还是处理一下，只有全部BrowserClient都释放了才置信
     * Shutdown事件
     **/
    std::list<BrowserClient*> g_client_list;
}
// ]

// bililive [
BrowserClient::BrowserClient(BrowserSource *bs_, bool sharing_avail, bool reroute_audio_) : 
    bs(bs_),
    sharing_available(sharing_avail),
    reroute_audio(reroute_audio_)
{
    g_client_list.push_back(this);

    if (!g_can_safely_cef_shutdown)
    {
        char* evt = getenv("g_can_safely_cef_shutdown");
        if (evt)
        {
            uint64 evt_l = std::stoull(evt);
            g_can_safely_cef_shutdown = (CefWaitableEvent*)evt_l;
        }
    }
    if (g_can_safely_cef_shutdown)
    {
        g_can_safely_cef_shutdown->Reset();
    }
}

void BrowserClient::DetachObsBrowserSource()
{
    base::AutoLock al(bs_lock);
    bs = nullptr;
}

// ]

BrowserClient::~BrowserClient()
{
#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED && USE_TEXTURE_COPY
	if (sharing_available) {
		obs_enter_graphics();
		gs_texture_destroy(texture);
		obs_leave_graphics();
	}
#endif

    // bililive [
    g_client_list.remove(this);

    if (g_client_list.empty())
    {
        // 所有浏览器源都退出了，置信允许CefShutdown
        if (g_can_safely_cef_shutdown)
        {
            QueueCEFTask(
                []()
                {
                    g_can_safely_cef_shutdown->Signal();
                    g_can_safely_cef_shutdown = nullptr;
                }
            );
        }
    }
    // ]
}

CefRefPtr<CefLoadHandler> BrowserClient::GetLoadHandler()
{
	return this;
}

CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
{
	return this;
}

CefRefPtr<CefDisplayHandler> BrowserClient::GetDisplayHandler()
{
	return this;
}

CefRefPtr<CefLifeSpanHandler> BrowserClient::GetLifeSpanHandler()
{
	return this;
}

CefRefPtr<CefContextMenuHandler> BrowserClient::GetContextMenuHandler()
{
	return this;
}

#if CHROME_VERSION_BUILD >= 3683
CefRefPtr<CefAudioHandler> BrowserClient::GetAudioHandler()
{
	return reroute_audio ? this : nullptr;
}
#endif

// bililive [
void BrowserClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
			        CefRefPtr<CefFrame> frame,
				TransitionType transition_type)
{
	if (frame->IsMain()) {
		std::string version = LIVEHIME_FULL_VERSION;
		std::string sname;
		{
			base::AutoLock al(bs_lock);
			if (bs) {
				sname = obs_source_get_name(bs->source);
			}
		}

		CefString eval_browser =
			"window.browser = {"
				"version: {"
					"android: false, BiliApp: true, mobile: false, "
					"ios: false, iPhone: false, iPad: false, MicroMessenger: false, "
					"webApp: false, pc_link: true"
				"},"
				"language: 'zh-CN',"
				"pc_link_scene: 'projection',"
				"app_version: '" + version + "'"
			"}";

		CefString url = frame->GetURL();
		frame->ExecuteJavaScript(eval_browser, url, 0);
	}
}
// ]

bool BrowserClient::OnBeforePopup(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>,
				  const CefString &, const CefString &,
				  WindowOpenDisposition, bool,
				  const CefPopupFeatures &, CefWindowInfo &,
				  CefRefPtr<CefClient> &, CefBrowserSettings &,
#if CHROME_VERSION_BUILD >= 3770
				  CefRefPtr<CefDictionaryValue> &,
#endif
				  bool *)
{
	/* block popups */
	return true;
}

// bililive [
bool BrowserClient::DoClose(CefRefPtr<CefBrowser> browser)
{
    blog(LOG_INFO, "obs-browser: client do close.");
    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void BrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
}
// ]

void BrowserClient::OnBeforeContextMenu(CefRefPtr<CefBrowser>,
					CefRefPtr<CefFrame>,
					CefRefPtr<CefContextMenuParams>,
					CefRefPtr<CefMenuModel> model)
{
	/* remove all context menu contributions */
	model->Clear();
}

bool BrowserClient::OnProcessMessageReceived(
	CefRefPtr<CefBrowser> browser,
#if CHROME_VERSION_BUILD >= 3770
	CefRefPtr<CefFrame>,
#endif
	CefProcessId, CefRefPtr<CefProcessMessage> message)
{
	const std::string &name = message->GetName();
	Json json;

	if (!bs) {
		return false;
	}

	if (name == "getCurrentScene") {
		OBSSource current_scene = obs_frontend_get_current_scene();
		obs_source_release(current_scene);

		if (!current_scene)
			return false;

		const char *name = obs_source_get_name(current_scene);
		if (!name)
			return false;

		json = Json::object{
			{"name", name},
			{"width", (int)obs_source_get_width(current_scene)},
			{"height", (int)obs_source_get_height(current_scene)}};

	} else if (name == "getStatus") {
		json = Json::object{
			{"recording", obs_frontend_recording_active()},
			{"streaming", obs_frontend_streaming_active()},
			{"recordingPaused", obs_frontend_recording_paused()},
			{"replaybuffer", obs_frontend_replay_buffer_active()}};

	} else if (name == "saveReplayBuffer") {
		obs_frontend_replay_buffer_save();
	} else {
		return false;
	}

	CefRefPtr<CefProcessMessage> msg =
		CefProcessMessage::Create("executeCallback");

	CefRefPtr<CefListValue> args = msg->GetArgumentList();
	args->SetInt(0, message->GetArgumentList()->GetInt(0));
	args->SetString(1, json.dump());

	SendBrowserProcessMessage(browser, PID_RENDERER, msg);

	return true;
}
#if CHROME_VERSION_BUILD >= 3578
void BrowserClient::GetViewRect(
#else
bool BrowserClient::GetViewRect(
#endif
	CefRefPtr<CefBrowser>, CefRect &rect)
{
	if (!bs) {
#if CHROME_VERSION_BUILD >= 3578
		rect.Set(0, 0, 16, 16);
		return;
#else
		return false;
#endif
	}

	rect.Set(0, 0, bs->width < 1 ? 1 : bs->width,
		 bs->height < 1 ? 1 : bs->height);
#if CHROME_VERSION_BUILD >= 3578
	return;
#else
	return true;
#endif
}

void BrowserClient::OnPaint(CefRefPtr<CefBrowser>, PaintElementType type,
			    const RectList &, const void *buffer, int width,
			    int height)
{
	if (type != PET_VIEW) {
		return;
	}

#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
	if (sharing_available) {
		return;
	}
#endif

    // bililive [
    base::AutoLock al(bs_lock);
    // ]

	if (!bs) {
		return;
	}

    // bililive [
    obs_enter_graphics();
    // ]
	if (bs->width != width || bs->height != height) {
    	// bililive [
		//obs_enter_graphics();
    	// ]
		bs->DestroyTextures();
    	// bililive [
		//obs_leave_graphics();
    	// ]
	}

	if (!bs->texture && width && height) {
    	// bililive [
		//obs_enter_graphics();
    	// ]
		bs->texture = gs_texture_create(width, height, GS_BGRA, 1,
						(const uint8_t **)&buffer,
						GS_DYNAMIC);
		bs->width = width;
		bs->height = height;
    	// bililive [
		//obs_leave_graphics();
    	// ]
	} else {
    	// bililive [
		//obs_enter_graphics();
    	// ]
		gs_texture_set_image(bs->texture, (const uint8_t *)buffer,
				     width * 4, false);
    	// bililive [
		//obs_leave_graphics();
    	// ]
    }
    // bililive [
    obs_leave_graphics();
    // ]
}

#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
void BrowserClient::OnAcceleratedPaint(CefRefPtr<CefBrowser>, PaintElementType,
				       const RectList &, void *shared_handle)
{
    // bililive [
    base::AutoLock al(bs_lock);
    // ]

	if (!bs) {
		return;
	}

    // bililive [
    obs_enter_graphics();
    // ]
	if (shared_handle != last_handle) {
    	// bililive [
		//obs_enter_graphics();
    	// ]
#if USE_TEXTURE_COPY
		gs_texture_destroy(texture);
		texture = nullptr;
#endif
		gs_texture_destroy(bs->texture);
		bs->texture = nullptr;

#if USE_TEXTURE_COPY
		texture = gs_texture_open_shared(
			(uint32_t)(uintptr_t)shared_handle);

		uint32_t cx = gs_texture_get_width(texture);
		uint32_t cy = gs_texture_get_height(texture);
		gs_color_format format = gs_texture_get_color_format(texture);

		bs->texture = gs_texture_create(cx, cy, format, 1, nullptr, 0);
#else
		bs->texture = gs_texture_open_shared(
			(uint32_t)(uintptr_t)shared_handle);
#endif
    	// bililive [
		//obs_leave_graphics();
    	// ]

		last_handle = shared_handle;
	}

#if USE_TEXTURE_COPY
	if (texture && bs->texture) {
    	// bililive [
		//obs_enter_graphics();
    	// ]
		gs_copy_texture(bs->texture, texture);
    	// bililive [
		//obs_leave_graphics();
    	// ]
	}
#endif
    // bililive [
    obs_leave_graphics();
    // ]
}
#endif

#if CHROME_VERSION_BUILD >= 3683
static speaker_layout GetSpeakerLayout(CefAudioHandler::ChannelLayout cefLayout)
{
	switch (cefLayout) {
	case CEF_CHANNEL_LAYOUT_MONO:
		return SPEAKERS_MONO; /**< Channels: MONO */
	case CEF_CHANNEL_LAYOUT_STEREO:
		return SPEAKERS_STEREO; /**< Channels: FL, FR */
	case CEF_CHANNEL_LAYOUT_2POINT1:
		return SPEAKERS_2POINT1; /**< Channels: FL, FR, LFE */
	case CEF_CHANNEL_LAYOUT_2_2:
	case CEF_CHANNEL_LAYOUT_QUAD:
	case CEF_CHANNEL_LAYOUT_4_0:
		return SPEAKERS_4POINT0; /**< Channels: FL, FR, FC, RC */
	case CEF_CHANNEL_LAYOUT_4_1:
		return SPEAKERS_4POINT1; /**< Channels: FL, FR, FC, LFE, RC */
	case CEF_CHANNEL_LAYOUT_5_1:
	case CEF_CHANNEL_LAYOUT_5_1_BACK:
		return SPEAKERS_5POINT1; /**< Channels: FL, FR, FC, LFE, RL, RR */
	case CEF_CHANNEL_LAYOUT_7_1:
	case CEF_CHANNEL_LAYOUT_7_1_WIDE_BACK:
	case CEF_CHANNEL_LAYOUT_7_1_WIDE:
		return SPEAKERS_7POINT1; /**< Channels: FL, FR, FC, LFE, RL, RR, SL, SR */
	}
	return SPEAKERS_UNKNOWN;
}
#endif

#if CHROME_VERSION_BUILD >= 4103
void BrowserClient::OnAudioStreamStarted(CefRefPtr<CefBrowser> browser,
					 const CefAudioParameters &params_,
					 int channels_)
{
	channels = channels_;
	channel_layout = (ChannelLayout)params_.channel_layout;
	sample_rate = params_.sample_rate;
	frames_per_buffer = params_.frames_per_buffer;
}

void BrowserClient::OnAudioStreamPacket(CefRefPtr<CefBrowser> browser,
					const float **data, int frames,
					int64_t pts)
{
	if (!bs) {
		return;
	}
	struct obs_source_audio audio = {};
	const uint8_t **pcm = (const uint8_t **)data;
	speaker_layout speakers = GetSpeakerLayout(channel_layout);
	int speaker_count = get_audio_channels(speakers);
	for (int i = 0; i < speaker_count; i++)
		audio.data[i] = pcm[i];
	audio.samples_per_sec = sample_rate;
	audio.frames = frames;
	audio.format = AUDIO_FORMAT_FLOAT_PLANAR;
	audio.speakers = speakers;
	audio.timestamp = (uint64_t)pts * 1000000LLU;
	obs_source_output_audio(bs->source, &audio);
}

void BrowserClient::OnAudioStreamStopped(CefRefPtr<CefBrowser> browser)
{
	if (!bs) {
		return;
	}
}

void BrowserClient::OnAudioStreamError(CefRefPtr<CefBrowser> browser,
				       const CefString &message)
{
	if (!bs) {
		return;
	}
}

static CefAudioHandler::ChannelLayout Convert2CEFSpeakerLayout(int channels)
{
	switch (channels) {
	case 1:
		return CEF_CHANNEL_LAYOUT_MONO;
	case 2:
		return CEF_CHANNEL_LAYOUT_STEREO;
	case 3:
		return CEF_CHANNEL_LAYOUT_2_1;
	case 4:
		return CEF_CHANNEL_LAYOUT_4_0;
	case 5:
		return CEF_CHANNEL_LAYOUT_4_1;
	case 6:
		return CEF_CHANNEL_LAYOUT_5_1;
	case 8:
		return CEF_CHANNEL_LAYOUT_7_1;
	default:
		return CEF_CHANNEL_LAYOUT_UNSUPPORTED;
	}
}

bool BrowserClient::GetAudioParameters(CefRefPtr<CefBrowser> browser,
				       CefAudioParameters &params)
{
	int channels = (int)audio_output_get_channels(obs_get_audio());
	params.channel_layout = Convert2CEFSpeakerLayout(channels);
	params.sample_rate = (int)audio_output_get_sample_rate(obs_get_audio());
	params.frames_per_buffer = kFramesPerBuffer;
	return true;
}
#elif CHROME_VERSION_BUILD >= 3683 && CHROME_VERSION_BUILD < 4103
void BrowserClient::OnAudioStreamStarted(CefRefPtr<CefBrowser> browser, int id,
					 int, ChannelLayout channel_layout,
					 int sample_rate, int)
{
	if (!bs) {
		return;
	}

	AudioStream &stream = bs->audio_streams[id];
	if (!stream.source) {
		stream.source = obs_source_create_private("audio_line", nullptr,
							  nullptr);
		obs_source_release(stream.source);

		obs_source_add_active_child(bs->source, stream.source);

		std::lock_guard<std::mutex> lock(bs->audio_sources_mutex);
		bs->audio_sources.push_back(stream.source);
	}

	stream.speakers = GetSpeakerLayout(channel_layout);
	stream.channels = get_audio_channels(stream.speakers);
	stream.sample_rate = sample_rate;
}

void BrowserClient::OnAudioStreamPacket(CefRefPtr<CefBrowser> browser, int id,
					const float **data, int frames,
					int64_t pts)
{
	if (!bs) {
		return;
	}

	AudioStream &stream = bs->audio_streams[id];
	struct obs_source_audio audio = {};

	const uint8_t **pcm = (const uint8_t **)data;
	for (int i = 0; i < stream.channels; i++)
		audio.data[i] = pcm[i];

	audio.samples_per_sec = stream.sample_rate;
	audio.frames = frames;
	audio.format = AUDIO_FORMAT_FLOAT_PLANAR;
	audio.speakers = stream.speakers;
	audio.timestamp = (uint64_t)pts * 1000000LLU;

	obs_source_output_audio(stream.source, &audio);
}

void BrowserClient::OnAudioStreamStopped(CefRefPtr<CefBrowser> browser, int id)
{
	if (!bs) {
		return;
	}

	auto pair = bs->audio_streams.find(id);
	if (pair == bs->audio_streams.end()) {
		return;
	}

	AudioStream &stream = pair->second;
	{
		std::lock_guard<std::mutex> lock(bs->audio_sources_mutex);
		for (size_t i = 0; i < bs->audio_sources.size(); i++) {
			obs_source_t *source = bs->audio_sources[i];
			if (source == stream.source) {
				bs->audio_sources.erase(
					bs->audio_sources.begin() + i);
				break;
			}
		}
	}
	bs->audio_streams.erase(pair);
}
#endif

void BrowserClient::OnLoadEnd(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame> frame,
			      int)
{
	if (!bs) {
		return;
	}

	if (frame->IsMain() && bs->css.length()) {
		std::string uriEncodedCSS =
			CefURIEncode(bs->css, false).ToString();

		std::string script;
		script += "const obsCSS = document.createElement('style');";
		script += "obsCSS.innerHTML = decodeURIComponent(\"" +
			  uriEncodedCSS + "\");";
		script += "document.querySelector('head').appendChild(obsCSS);";

		frame->ExecuteJavaScript(script, "", 0);
	}
}

bool BrowserClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
#if CHROME_VERSION_BUILD >= 3282
				     cef_log_severity_t level,
#endif
				     const CefString &message,
				     const CefString &source, int line)
{
#if CHROME_VERSION_BUILD >= 3282
    // bililive[
    if (level < LOGSEVERITY_WARNING)
        //return false;
    // ]

    // bililive[
#ifdef _DEBUG
        return false;
#else
        return true;
#endif
    // ]
#endif

    // bililive[
    CefString url;
    if (browser && browser->GetMainFrame())
    {
        url = browser->GetMainFrame()->GetURL();
    }

	blog(LOG_INFO, "obs-browser: %s (source: %s:%d), url: %s",
	     message.ToString().c_str(), source.ToString().c_str(), line, url.ToString().c_str());
    // ]
	return false;
}
