#pragma once

#pragma comment(lib, "DiscordRichPresense.lib")

#include "../DiscordRPC/discord_register.h"
#include "../DiscordRPC/discord_rpc.h"

class ENGINE_API DiscordRPC final
{
	const char* current_level_name{};
	const char* active_task_text{};
	int64_t start_time{};

public:
	DiscordRPC() = default;
	~DiscordRPC();

	void Init();
	void Update(const char* level_name = nullptr);
	void Set_active_task_text(const char* txt) {
		active_task_text = txt;
		Update();
	}
};

extern ENGINE_API DiscordRPC Discord;
