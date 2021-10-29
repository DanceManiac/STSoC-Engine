#include "stdafx.h"

#include "DiscordRPC.hpp"

ENGINE_API DiscordRPC Discord;

void DiscordRPC::Init()
{
	DiscordEventHandlers nullHandlers{};
	Discord_Initialize("862971629810221086", &nullHandlers, TRUE, nullptr, 0);

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

	if (active_task_text) {
		task_txt = StringToUTF8(active_task_text);
		presenseInfo.state = task_txt.c_str(); //Активное задание
	}

	if (level_name) 
		current_level_name = level_name;

	if (current_level_name) {
		lname = StringToUTF8(current_level_name);
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
