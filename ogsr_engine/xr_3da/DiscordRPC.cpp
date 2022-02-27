#include "stdafx.h"

#include "DiscordRPC.hpp"

ENGINE_API DiscordRPC Discord;

void DiscordRPC::Init()
{
	DiscordEventHandlers nullHandlers{};
	Discord_Initialize(READ_IF_EXISTS(pSettings, r_string, "rpc_ettings", "discordrpc_appid", "862971629810221086"), &nullHandlers, TRUE, nullptr, 0);

	start_time = time(nullptr);
}

DiscordRPC::~DiscordRPC()
{
	Discord_ClearPresence();
	Discord_Shutdown();
}


void DiscordRPC::Update(const char* level_name)
{
	DiscordRichPresence presenseInfo{};

	presenseInfo.startTimestamp = start_time; //время с момента запуска
	presenseInfo.largeImageKey = "main_image"; //большая картинка
	presenseInfo.smallImageKey = "main_image_small"; //маленькая картинка
	presenseInfo.smallImageText = Core.GetEngineVersion(); //версия движка на маленькой картинке

	std::string task_txt, lname, lname_and_task;
	
	using namespace std;
	if (active_task_text) {
		const bool show_text = READ_IF_EXISTS(pSettings, r_bool, "rpc_ettings", "show_current_task", true);
		task_txt = StringToUTF8(string(string("Задание: ") + string(show_text ? active_task_text : "СКРЫТО")).c_str());
		presenseInfo.state = task_txt.c_str(); //Активное задание
	}

	if (level_name) 
		current_level_name = level_name;

	if (current_level_name) {
		const bool show_text = READ_IF_EXISTS(pSettings, r_bool, "rpc_ettings", "show_current_level", true);
		lname = StringToUTF8(string(string("Уровень: ") + string(show_text ? current_level_name : "СКРЫТО")).c_str());
		presenseInfo.details = lname.c_str(); //название уровня
	}

	if (!lname.empty()) {
		lname_and_task = lname;
		if (!task_txt.empty()) {
			lname_and_task += " | ";
			lname_and_task += task_txt;
		}
	}
	else if (!task_txt.empty()) {
		lname_and_task = task_txt;
	}

	if (!lname_and_task.empty())
		presenseInfo.largeImageText = lname_and_task.c_str(); //название уровня + активное задание на большой картинке

	Discord_UpdatePresence(&presenseInfo);
}
