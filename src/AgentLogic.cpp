#include "AgentLogic.h"

juce::Array<AgentCommand> AgentLogic::interpretInstruction(const juce::String& instruction)
{
    juce::Array<AgentCommand> commands;
    AgentCommand cmd;
    juce::String input = instruction.toLowerCase();

    // 1. Detect Role
    if (input.contains("ドラム") || input.contains("ビート") || input.contains("drums")) {
        cmd.role = MusicalRole::Drums;
        cmd.targetTrackName = "Drums";
    } else if (input.contains("ベース") || input.contains("低音") || input.contains("bass")) {
        cmd.role = MusicalRole::Bass;
        cmd.targetTrackName = "Bass";
    } else if (input.contains("メロディ") || input.contains("melody")) {
        cmd.role = MusicalRole::Melody;
        cmd.targetTrackName = "Melody";
    } else if (input.contains("コード") || input.contains("和音") || input.contains("chords")) {
        cmd.role = MusicalRole::Chords;
        cmd.targetTrackName = "Chords";
    } else if (input.contains("パッド") || input.contains("pad")) {
        cmd.role = MusicalRole::Pad;
        cmd.targetTrackName = "Pad";
    } else if (input.contains("効果音") || input.contains("fx") || input.contains("se")) {
        cmd.role = MusicalRole::FX;
        cmd.targetTrackName = "FX";
    } else {
        // Default to Melody if unspecified, or return empty if strict
        cmd.role = MusicalRole::Melody;
        cmd.targetTrackName = "Melody";
    }

    // 2. Detect Action
    if (input.contains("作り直し") || input.contains("置き換え") || input.contains("replace")) {
        cmd.action = AgentAction::Replace;
    } else if (input.contains("続き") || input.contains("延長") || input.contains("伸ばす") || input.contains("extend")) {
        cmd.action = AgentAction::Extend;
    } else if (input.contains("消す") || input.contains("クリア") || input.contains("削除") || input.contains("clear")) {
        cmd.action = AgentAction::ClearRegion;
    } else if (input.contains("人間") || input.contains("揺らす") || input.contains("humanize")) {
        cmd.action = AgentAction::Humanize;
    } else {
        cmd.action = AgentAction::Generate; // Default
    }

    // 3. Detect Style
    if (input.contains("暗い") || input.contains("ダーク") || input.contains("dark")) {
        cmd.style = "dark";
    } else if (input.contains("しずか") || input.contains("アンビエント") || input.contains("ambient")) {
        cmd.style = "ambient";
    } else if (input.contains("激しい") || input.contains("エネルギッシュ") || input.contains("energetic")) {
        cmd.style = "energetic";
    } else if (input.contains("ローファイ") || input.contains("lofi")) {
        cmd.style = "lofi";
    }

    // 4. Detect Bars (Regex-like manual parsing for simplicity without <regex> dependency if possible, 
    // but JUCE doesn't have built-in regex in core module easily accessible without extra headers. 
    // Let's use simple string parsing for "X-Y小節" or "X~Y小節")
    
    // Simple parser for "X-Y" or "X~Y"
    int dashIndex = input.indexOf("-");
    if (dashIndex == -1) dashIndex = input.indexOf("~");
    
    if (dashIndex > 0)
    {
        // Try to read numbers around the dash
        juce::String pre = input.substring(0, dashIndex).trim();
        juce::String post = input.substring(dashIndex + 1).trim();
        
        // Extract number from end of pre
        int start = pre.getTrailingIntValue();
        // Extract number from start of post
        int end = post.getIntValue();
        
        if (start > 0 && end >= start)
        {
            cmd.barsStart = start;
            cmd.barsEnd = end;
        }
    }

    commands.add(cmd);
    return commands;
}
