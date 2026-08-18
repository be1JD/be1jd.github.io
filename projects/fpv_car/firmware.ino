/*
/*
    Author : JD Patel
    Date Created : 26-04-2026
    Features : Web_Controller + Real Time Control
    To Change :- 
            - 
            - Write Pulse widthin Time instead of Angle for steering ( better resolution but more confusing )

    To Test :-  
            - Non Volatile parameter storage. On ESP32, Preferences (NVS - Non Volatile Storage).
            - 
*/
/* Private Includes Start */

#include <WiFi.h>               // Wifi Support
#include <ESPAsyncWebServer.h>  // Web Server & Websocket Support
#include <AsyncTCP.h>           // TCP Networking Stack
#include <ESP32Servo.h>         // Servo Handling API's
#include <Preferences.h>        // For Non Volatile Storage

/* Private Includes End */

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport"
        content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no, viewport-fit=cover">
    <meta name="theme-color" content="#000000">
    <meta name="application-name" content="JetRay">
    <meta name="apple-mobile-web-app-title" content="JetRay">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="display" content="fullscreen">
    <link rel="manifest"
        href='data:application/manifest+json,%7B%22name%22%3A%22JetRay%20FPV%20Controller%22%2C%22short_name%22%3A%22JetRay%22%2C%22start_url%22%3A%22%2F%22%2C%22scope%22%3A%22%2F%22%2C%22display%22%3A%22fullscreen%22%2C%22orientation%22%3A%22landscape%22%2C%22background_color%22%3A%22%23000000%22%2C%22theme_color%22%3A%22%23000000%22%7D'>
    <title>JetRay</title>
    <style>
        :root {
            /* Theme colors: edit this block to reskin the controller. */
            --theme-accent: #32d8f4;
            --theme-accent-soft: #95edff33;
            --theme-warning: #ffd66f;
            --theme-danger: #ff4f62;
            --theme-success: #2cff95;
            --theme-text: #eef;
            --theme-muted: #9fc7d4;
            --theme-surface: #06101866;
            --theme-surface-strong: #061018e8;
            --theme-input: #061018aa;
            --theme-control: #10222f;
            --theme-control-soft: #10222fc7;
            --theme-track: #6a6a6a53;
            --theme-border-color: #949494;
            --theme-shadow: #1de3ff33;
            --theme-page-overlay: rgba(0, 0, 0, .1);
            --theme-screen-gradient: linear-gradient(rgba(0, 0, 0, .18), rgba(0, 0, 0, .62));
            --theme-bg: #000;
            --button-border: 1px solid var(--theme-border-color);
            --speed-bar-color: var(--theme-color-2);
            --steer-bar-color: var(--theme-color-2);
            --theme-color-1: var(--theme-border-color);
            --theme-color-2: var(--theme-accent);
            --theme-color-3: var(--theme-track);
            --theme-color-4: var(--theme-warning);
            color-scheme: dark;
            --app-height: 100dvh;
            --app-width: 100vw;
            --panel-gap: clamp(8px, 1.8vw, 14px);
            --stat-width: clamp(72px, 14vw, 96px);
            --side-button: clamp(58px, 12vw, 72px);
            --drive-width: clamp(64px, 13vw, 82px);
            --drive-height: clamp(120px, 28vh, 168px);
            --brake-height: clamp(102px, 22vh, 146px);
            --speed-panel-height: clamp(78px, 14vh, 104px);
            --steer-panel-width: min(200px, 35vw);
            --speed-slider-width: min(100%, 240px);
            --steer-track-height: clamp(62px, 11vh, 78px);
            --steer-track-pad-left: clamp(14px, 3vw, 26px);
            --steer-track-pad-right: clamp(8px, 1.8vw, 14px);
            --steer-track-thickness: clamp(14px, 2.2vh, 18px);
            --steer-thumb-size: clamp(28px, 4vh, 34px);
            --speed-slider-length: 180px;
            --speed-slider-track: 28px;
            --speed-thumb-size: 24px;
            --text-lg: clamp(18px, 4vw, 22px);
            --text-md: clamp(16px, 3vw, 20px);
            --text-sm: clamp(10px, 2vw, 12px);
            --edge-pad: 10px;
            /* settings panel variables */
            --settings-bg: var(--theme-surface);
            --settings-border: var(--button-border);
        }

        * {
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
        }

        html,
        body {
            margin: 0;
            min-height: 100svh;
            height: var(--app-height);
            overflow: hidden;
            background: var(--theme-bg);
            font: 14px Arial, sans-serif;
            color: var(--theme-text);
        }

        body {
            position: fixed;
            inset: 0;
            user-select: none;
            overscroll-behavior: none;
            /* touch-action: pan-x pan-y; */
            touch-action: none;
        }

        #video_feed {
            transform: rotate(90deg);
            position: fixed;
            inset: 0;
            width: 100vh;
            /* width: 100%; */
            /* height: 100%;  */
            justify-self: center;
            align-self: center;
            object-fit: fit-content;
            background: #111;
            pointer-events: none;
        }

        main {
            position: relative;
            z-index: 1;
            min-height: 100svh;
            height: var(--app-height);
            padding: calc(var(--edge-pad) + env(safe-area-inset-top)) calc(var(--edge-pad) + env(safe-area-inset-right)) calc(var(--edge-pad) + env(safe-area-inset-bottom)) calc(var(--edge-pad) + env(safe-area-inset-left));
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            gap: var(--panel-gap);
            background: var(--theme-screen-gradient);
        }

        .nav {
            display: flex;
            justify-content: space-between;
            gap: var(--panel-gap);
            flex-wrap: wrap;
            flex: 0 1 auto;
        }

        .nav_bar {
            display: flex;
            gap: var(--panel-gap);
            flex-wrap: wrap;
        }

        .p {
            /* border: 1px solid #1de3ff8a; */
            border: var(--button-border);
            background: var(--theme-surface);
            box-shadow: 0 0 0 1px var(--theme-shadow) inset, 0 0 16px #1de3ff22;
            border-radius: 14px;
            backdrop-filter: blur(6px);
            padding: 10px 12px;
        }

        .stat_box {
            min-width: 150px;
        }

        .stat_box b {
            display: block;
            font-size: 20px;
            margin-top: 4px;
        }

        .stat_info {
            display: flex;
            align-items: center;
            gap: 8px;
            color: #9fc7d4;
            font-size: 11px;
            margin-top: 6px;
        }

        #stat_led {
            width: 9px;
            height: 9px;
            border-radius: 50%;
            background: #ff4f62;
            box-shadow: 0 0 8px #ff4f62;
        }

        .ctrl_box {
            min-width: var(--stat-width);
            text-align: center;
        }

        .ctrl_box b {
            display: block;
            font-size: var(--text-lg);
        }

        .battery_box {
            min-width: 112px;
            text-align: left;
        }

        .battery_reading {
            display: flex;
            justify-content: space-between;
            gap: 10px;
            margin-top: 4px;
            color: var(--theme-muted);
            font-size: 11px;
        }

        .battery_reading b {
            font-size: var(--text-md);
            color: var(--theme-text);
        }

        .top-tools {
            position: relative;
        }

        .debug_btn {
            padding: 0 12px;
            height: 42px;
            color: #fff;
            background: #0006;
            border: var(--button-border);
            border-radius: 12px;
        }

        .disp_none {
            display: none;
        }

        #db_panel {
            border: var(--button-border);
            background: #06101866;
            box-shadow: 0 0 0 1px #1de3ff33 inset, 0 0 16px #1de3ff22;
            border-radius: 14px;
            position: absolute;
            top: calc(100% + 8px);
            right: 0;
            z-index: 5;
            width: min(220px, calc(100vw - 24px));
            padding: 8px 10px;
            font: 10px/1.25 Consolas, monospace;
            color: #7effb8;
        }

        #db_panel i {
            display: flex;
            justify-content: space-between;
            color: #9fc7d4;
            font-style: normal;
            margin-bottom: 4px;
        }

        #db_panel p {
            margin: 0 0 4px;
            color: #2cff95;
            word-break: break-word;
        }

        #db_panel pre {
            margin: 0;
            max-height: 54px;
            overflow: auto;
            white-space: pre-wrap;
        }

        .ctrl_sec {
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            gap: var(--panel-gap);
            min-height: 0;
            flex-wrap: wrap;
            flex: 1 1 auto;
            align-content: flex-end;
        }

        .left_ctrls {
            display: flex;
            flex-direction: column;
            gap: var(--panel-gap);
            align-items: stretch;
            flex: 1;
            min-width: 0;
            max-width: min(58vw, 400px);
        }

        .right_ctrls {
            display: flex;
            /* gap: var(--panel-gap); */
            gap: clamp(12px, 4vw, 40px);
            align-items: flex-end;
            margin-left: auto;
            margin-right: 5%;
            flex-wrap: nowrap;
        }

        .u {
            text-align: center;
            color: #9fc7d4;
            font-size: 11px;
        }

        .u b {
            display: block;
            font-size: var(--text-md);
            color: #eef;
        }

        .v {
            width: 60px;
            height: calc(var(--speed-slider-length) + 58px);
            max-height: 100%;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 12px;
            border: none;
            box-shadow: none;
        }

        .vw {
            
            width: 40px;
            height: var(--speed-slider-length);
            display: flex;
            align-items: center;
            justify-content: center;
        }

        #speed_bar {
            appearance: none;
            -webkit-appearance: none;

            width: var(--speed-slider-length);
            height: 18px;

            transform: rotate(-90deg);

            background: transparent;
        }

        /* Track */
        #speed_bar::-webkit-slider-runnable-track {
            height: var(--speed-slider-track);
            background: #6a6a6a53;
            border-radius: 999px;
            border: 1px solid #95edff33;
        }

        /* Thumb */
        #speed_bar::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: var(--speed-thumb-size);
            height: var(--speed-thumb-size);
            margin-top: calc((var(--speed-slider-track) - var(--speed-thumb-size)) / 2);

            border-radius: 50%;
            /* background: #32d8f4; */
            background: var(--speed-bar-color);
            border: none;

            box-shadow: 0 0 0 3px #071019, 0 0 12px #32d8f455;
        }

        .w {
            width: var(--steer-panel-width);
            min-width: 0;
            padding: 10px 12px 12px;
            flex: 1 1 auto;
            border: none;
            box-shadow: none;
            background: transparent;
            backdrop-filter: none;
        }

        .steer_bar {
            width: calc(100% - var(--steer-track-pad-left) - var(--steer-track-pad-right));
            height: var(--steer-track-height);
            margin: 10px var(--steer-track-pad-right) 4px var(--steer-track-pad-left);
            accent-color: #32d8f4;
            appearance: none;
            -webkit-appearance: none;
            background: transparent;
        }

        .steer_bar::-webkit-slider-runnable-track {
            height: var(--steer-track-thickness);
            /* background: #6a6a6a; */
            background: #6a6a6a53;
            border-radius: 999px;
            border: 1px solid #95edff33;
            box-shadow: inset 0 0 0 1px #00000030;
        }

        .steer_bar::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: var(--steer-thumb-size);
            height: var(--steer-thumb-size);
            margin-top: calc((var(--steer-track-thickness) - var(--steer-thumb-size)) / 2);
            border-radius: 50%;
            border: none;
            /* background: #32d8f4; */
            background: var(--steer-bar-color);
            box-shadow: 0 0 0 3px #071019, 0 0 12px #32d8f455;
        }

        .steer_bar::-moz-range-track {
            height: var(--steer-track-thickness);
            background: #6a6a6a;
            border-radius: 999px;
            border: 1px solid #95edff33;
            box-shadow: inset 0 0 0 1px #00000030;
        }

        .steer_bar::-moz-range-progress {
            height: var(--steer-track-thickness);
            background: #6a6a6a;
            border-radius: 999px;
        }

        .steer_bar::-moz-range-thumb {
            width: var(--steer-thumb-size);
            height: var(--steer-thumb-size);
            border-radius: 50%;
            border: none;
            background: #32d8f4;
            box-shadow: 0 0 0 3px #071019, 0 0 12px #32d8f455;
        }

        .steer_reverse {
            transform: rotateY(180deg);
        }

        .c {
            display: flex;
            flex-direction: column;
            gap: 10px;
        }

        .a {
            width: var(--side-button);
            height: var(--side-button);
            padding: 0 4px;
            color: #fff;
            background: #06101855;
            border: var(--button-border);
            border-radius: 18px;
            font-size: var(--text-sm);
        }

        .a small {
            display: block;
            font-size: clamp(8px, 1.7vw, 9px);
            color: #9fc7d4;
        }

        .ps {
            display: flex;
            /* gap: var(--panel-gap); */
            gap: clamp(10px, 3vw, 25px);
            align-items: flex-end;
        }

        .pc {
            display: flex;
            flex-direction: column;
            /* gap: var(--panel-gap); */
            gap: 10px
        }

        .y {
            width: var(--drive-width);
            height: var(--drive-height);
            position: relative;
            overflow: hidden;
            color: #fff;
            background: linear-gradient(rgba(255, 255, 255, .08), rgba(255, 255, 255, .02));
            border: var(--button-border);
            border-radius: 20px;
            font: 700 clamp(14px, 2.8vw, 16px) Arial;
            flex: 0 0 auto;
        }

        .y:before {
            content: "";
            position: absolute;
            inset: clamp(10px, 2.4vh, 14px) clamp(12px, 2.8vw, 16px);
            background: repeating-linear-gradient(180deg, var(--theme-color-1) 0 2px, #0000 2px 14px);
            /* background: repeating-linear-gradient(180deg, #1de3ffb8 0 2px, #0000 2px 14px); */
            border-radius: 12px;
        }

        .y.red {
            height: var(--brake-height);
            border-color: #ff4f62;
        }

        .y.red:before {
            background: repeating-linear-gradient(180deg, #ff4f62c8 0 2px, #0000 2px 14px);
        }

        .y span,
        .a span {
            position: relative;
            z-index: 1;
        }

        .on {
            transform: scale(.98);
            background: #1de3ff2a;
        }

        /* Setting Panel Styling Start */
        /* FULLSCREEN SETTINGS */

        .settings_overlay {
            position: fixed;
            inset: 0;
            z-index: 999;
            /* background: radial-gradient(circle at center, rgba(20, 40, 60, .96), rgba(0, 0, 0, .98)); */
            background: radial-gradient(circle at center, rgba(20, 40, 60, .6), rgba(0, 0, 0, .7));
            backdrop-filter: blur(1px);
            display: none;
            overflow: hidden;
        }

        .settings_overlay.show {
            display: block;
        }

        /* CLOSE BUTTON */
        .close_settings_btn {
            position: absolute;
            top: 20px;
            right: 20px;
            width: 38px;
            height: 38px;
            border-radius: 50%;
            border: 1px solid rgb(189, 119, 119);
            /* border: 1px solid red; */
            background: black;
            font-size: 16px;
            z-index: 20;
        }

        /* HOME MENU */
        .settings_home {
            position: absolute;
            inset: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 18px;
        }

        /* BIG BUTTONS */
        .settings_card {
            width: min(240px, 82vw);
            height: 48px;
            border-radius: 10px;
            border: var(--button-border);
            background: rgba(6, 16, 24, .78);
            color: white;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 18px;
            transition: .2s;
        }

        .settings_card span {
            font-size: 18px;
        }

        .settings_card:hover {
            transform: scale(1.04);
            border-color: #32d8f4;
            box-shadow: 0 0 24px #1de3ff55;
        }

        /* PAGE AREA */
        .settings_pages {
            width: min(720px, 92vw);
            margin-top: 5px;
        }

        .settings_page {
            width: 80%;
            min-height: 360px;
            padding: 24px;
            border-radius: 24px;
            background: var(--theme-page-overlay);
            border: var(--settings-border);
            backdrop-filter: blur(2px);
            position: absolute;
            z-index: 1002;
        }

        .settings_page h2 {
            margin-top: 0;
            color: var(--theme-accent);
        }

        .settings_grid {
            display: grid;
            grid-template-columns: repeat(2, minmax(0, 1fr));
            gap: 12px;
        }

        .setting_item {
            display: grid;
            gap: 6px;
            min-width: 0;
            color: var(--theme-muted);
            font-size: 12px;
        }

        .setting_item span {
            display: flex;
            justify-content: space-between;
            gap: 8px;
        }

        .setting_item b {
            color: var(--theme-text);
            font-weight: 700;
        }

        .setting_item input {
            width: 100%;
            min-width: 0;
        }

        .setting_item input[type="number"],
        .setting_item input[type="text"],
        .setting_item input[type="password"],
        .setting_item select {
            height: 36px;
            padding: 0 10px;
            color: var(--theme-text);
            background: var(--theme-input);
            border: var(--button-border);
            border-radius: 8px;
            outline: none;
        }

        .setting_item select {
            appearance: none;
            -webkit-appearance: none;
            cursor: pointer;
            padding-right: 38px;
            background-color: var(--theme-control-soft);
            background-image:
                linear-gradient(45deg, transparent 50%, var(--theme-accent) 50%),
                linear-gradient(135deg, var(--theme-accent) 50%, transparent 50%),
                linear-gradient(to right, var(--theme-accent-soft), var(--theme-accent-soft));
            background-position:
                calc(100% - 20px) 15px,
                calc(100% - 14px) 15px,
                calc(100% - 34px) 8px;
            background-size: 6px 6px, 6px 6px, 1px 20px;
            background-repeat: no-repeat;
            box-shadow: 0 0 0 1px var(--theme-shadow) inset;
            transition: border-color .15s ease, box-shadow .15s ease, background-color .15s ease;
        }

        .setting_item select:hover,
        .setting_item select:focus {
            border-color: var(--theme-accent);
            box-shadow: 0 0 0 1px var(--theme-accent-soft) inset, 0 0 14px var(--theme-shadow);
        }

        .setting_item select option {
            color: var(--theme-text);
            background: var(--theme-control);
        }

        .settings_group,
        .settings_actions {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            margin-top: 14px;
        }

        .settings_group button,
        .settings_actions button,
        .save_dirty_btn {
            min-height: 38px;
            padding: 0 14px;
            color: var(--theme-text);
            background: var(--theme-control);
            border: var(--button-border);
            border-radius: 8px;
        }

        .settings_group button:disabled,
        .settings_actions button:disabled,
        .save_dirty_btn:disabled {
            cursor: wait;
            opacity: .62;
            filter: saturate(.7);
        }

        .settings_group button.on {
            border-color: var(--theme-accent);
            color: var(--theme-accent);
            box-shadow: 0 0 0 1px var(--theme-accent-soft) inset, 0 0 14px var(--theme-shadow);
        }

        .settings_note {
            margin: 12px 0 0;
            color: var(--theme-muted);
            font-size: 12px;
            line-height: 1.35;
        }

        .dirty_banner {
            position: fixed;
            right: max(12px, env(safe-area-inset-right));
            bottom: max(12px, env(safe-area-inset-bottom));
            z-index: 1001;
            display: flex;
            align-items: center;
            gap: 10px;
            padding: 8px 10px;
            border: var(--button-border);
            border-radius: 10px;
            background: var(--theme-surface-strong);
            box-shadow: 0 0 16px var(--theme-shadow);
            color: var(--theme-text);
            font-size: 12px;
        }

        .back_btn {
            margin-bottom: 20px;
            padding: 10px 18px;
            border: none;
            border-radius: 12px;
            background: var(--theme-control);
            color: var(--theme-accent);
            border: var(--button-border);
            font-size: 15px;
        }

        .back_btn:active {
            transform: scale(.96);
        }

        /* Settings Panel Styling End */

        .rotate_prompt,
        .fullscreen_hint {
            position: fixed;
            inset: 0;
            z-index: 0;
            display: none;
            place-items: center;
            padding: calc(20px + env(safe-area-inset-top)) calc(18px + env(safe-area-inset-right)) calc(20px + env(safe-area-inset-bottom)) calc(18px + env(safe-area-inset-left));
            pointer-events: none;
            background:
                radial-gradient(circle at 50% 50%, rgba(50, 216, 244, .18), transparent 34%),
                rgba(0, 0, 0, .18);
            color: var(--theme-text);
            text-align: center;
            opacity: .76;
            filter: blur(.15px);
        }

        .rotate_prompt_card,
        .fullscreen_hint_card {
            display: grid;
            justify-items: center;
            gap: 10px;
            max-width: min(320px, 86vw);
            padding: 18px;
            border: 1px solid rgba(255, 255, 255, .12);
            border-radius: 18px;
            background: rgba(0, 0, 0, .2);
            box-shadow: 0 0 42px rgba(50, 216, 244, .18);
            backdrop-filter: blur(4px);
            animation: notice_float 2.4s ease-in-out infinite;
        }

        .rotate_prompt h2,
        .fullscreen_hint h2 {
            margin: 0;
            color: var(--theme-text);
            font-size: clamp(18px, 5vw, 28px);
            letter-spacing: 0;
        }

        .rotate_prompt p,
        .fullscreen_hint p {
            margin: 0;
            color: var(--theme-muted);
            font-size: clamp(12px, 3.3vw, 15px);
            line-height: 1.45;
        }

        .rotate_art {
            position: relative;
            width: 124px;
            height: 168px;
            margin: 0;
            opacity: .9;
            animation: notice_pulse 1.8s ease-in-out infinite;
        }

        .rotate_phone {
            position: absolute;
            left: 50%;
            top: 50%;
            width: 72px;
            height: 128px;
            transform: translate(-50%, -50%);
            border: 4px solid #fff;
            border-radius: 13px;
            box-shadow: 0 0 24px rgba(255, 255, 255, .18);
        }

        .rotate_phone::before,
        .rotate_phone::after {
            content: "";
            position: absolute;
            left: 50%;
            transform: translateX(-50%);
            background: #fff;
        }

        .rotate_phone::before {
            top: 8px;
            width: 20px;
            height: 3px;
            border-radius: 999px;
        }

        .rotate_phone::after {
            bottom: 8px;
            width: 30px;
            height: 3px;
            border-radius: 999px;
        }

        .rotate_arrow {
            position: absolute;
            width: 56px;
            height: 56px;
            border: 5px solid #fff;
            border-left-color: transparent;
            border-bottom-color: transparent;
            border-radius: 50%;
        }

        .rotate_arrow::after {
            content: "";
            position: absolute;
            width: 0;
            height: 0;
            border: 10px solid transparent;
        }

        .rotate_arrow.top {
            left: 6px;
            top: 8px;
            transform: rotate(-20deg);
        }

        .rotate_arrow.top::after {
            right: -9px;
            top: -13px;
            border-left-color: #fff;
            transform: rotate(42deg);
        }

        .rotate_arrow.bottom {
            right: 4px;
            bottom: 9px;
            transform: rotate(160deg);
        }

        .rotate_arrow.bottom::after {
            right: -9px;
            top: -13px;
            border-left-color: #fff;
            transform: rotate(42deg);
        }

        .fullscreen_hint {
            align-items: end;
            padding-bottom: max(22px, env(safe-area-inset-bottom));
            background:
                radial-gradient(circle at 50% 82%, rgba(50, 216, 244, .2), transparent 26%),
                rgba(0, 0, 0, .06);
            opacity: .7;
        }

        .fullscreen_hint.show {
            display: grid;
        }

        .tap_icon {
            width: 60px;
            height: 60px;
            display: grid;
            place-items: center;
            border: 2px solid var(--theme-accent);
            border-radius: 50%;
            color: var(--theme-accent);
            box-shadow: 0 0 28px var(--theme-shadow);
            font-size: 22px;
            font-weight: 700;
            animation: notice_pulse 1.8s ease-in-out infinite;
        }

        @keyframes notice_float {
            0%,
            100% {
                transform: translateY(0);
            }

            50% {
                transform: translateY(-8px);
            }
        }

        @keyframes notice_pulse {
            0%,
            100% {
                opacity: .62;
                transform: scale(.96);
            }

            50% {
                opacity: 1;
                transform: scale(1);
            }
        }

        @media (orientation: portrait) {
            .rotate_prompt {
                display: grid;
            }

            main {
                padding-top: calc(12px + env(safe-area-inset-top));
                padding-right: calc(12px + env(safe-area-inset-right));
                padding-bottom: calc(12px + env(safe-area-inset-bottom));
                padding-left: calc(12px + env(safe-area-inset-left));
            }

            .ctrl_sec {
                flex-direction: column;
                align-items: stretch;
            }

            .left_ctrls,
            .right_ctrls,
            .nav,
            .nav_bar {
                width: 100%;
                max-width: 100%;
            }

            .right_ctrls {
                justify-content: space-between;
                margin-left: 0;
                margin-right: 0;
            }

            .w {
                width: 100%;
            }
        }

        @media (max-width: 900px) {
            :root {
                --steer-panel-width: min(100%, 420px);
            }

            .ctrl_sec {
                justify-content: center;
            }

            .left_ctrls {
                max-width: 100%;
            }
            .right_ctrls {
                margin-left: 0;
            }
        }

        @media (max-width: 720px),
        (max-height: 520px) and (orientation: landscape) {
            :root {
                --panel-gap: clamp(6px, 1.5vw, 10px);
                --stat-width: clamp(62px, 12vw, 84px);
                --side-button: clamp(50px, 10vw, 62px);
                --drive-width: clamp(54px, 11vw, 68px);
                --drive-height: clamp(102px, 24vh, 138px);
                --brake-height: clamp(90px, 20vh, 118px);
                --speed-panel-height: clamp(70px, 13vh, 90px);
                --speed-slider-length: clamp(118px, 33vh, 160px);
                --steer-panel-width: min(250px, 46vw);
                --steer-track-height: clamp(54px, 10vh, 64px);
                --steer-track-thickness: clamp(12px, 2vh, 16px);
                --steer-thumb-size: clamp(24px, 3.4vh, 30px);
                --text-lg: clamp(16px, 3vw, 19px);
                --text-md: clamp(14px, 2.6vw, 18px);
                --text-sm: clamp(9px, 1.8vw, 11px);
                --edge-pad: 8px;
            }

            .stat_box {
                min-width: 132px;
            }

            .p,
            #db_panel {
                border-radius: 12px;
            }

            .v,
            .w,
            .a,
            .y {
                border-radius: 16px;
            }
        }

        @media (max-height: 480px) and (orientation: landscape) {
            :root {
                --panel-gap: 6px;
                --side-button: 46px;
                --drive-width: 70px;
                --drive-height: 92px;
                --brake-height: 82px;
                --speed-panel-height: 62px;
                --speed-slider-length: clamp(96px, 28vh, 132px);
                --steer-track-height: 68px;
                --steer-track-pad-left: 10px;
                --steer-track-pad-right: 6px;
                --steer-track-thickness: 58px;
                --steer-thumb-size: 45px;
                --text-lg: 15px;
                --text-md: 13px;
                --text-sm: 8px;
                --edge-pad: 6px;
            }

            .p {
                padding: 8px 10px;
            }

            .debug_btn {
                height: 36px;
                padding: 0 10px;
            }
        }
    </style>
</head>

<body>
    <!-- <img id="video_feed" src="http://192.168.4.2:81/stream" alt="https://preview.redd.it/whats-your-prettiest-rc-pictures-v0-95pxwgk60y4e1.jpeg?width=640&crop=smart&auto=webp&s=ebb69a8fb434c3a520a3deec41a4ade1cfb0ce37" onerror="this.style.display='none'"> -->
    <!-- <img id="video_feed"
        src="https://preview.redd.it/whats-your-prettiest-rc-pictures-v0-95pxwgk60y4e1.jpeg?width=640&crop=smart&auto=webp&s=ebb69a8fb434c3a520a3deec41a4ade1cfb0ce37"
        alt="" onerror="this.style.display='none'"> -->
    <!-- Search for :- lanscape image taken from rc car -->
    <div id="rotate_prompt" class="rotate_prompt" aria-live="polite">
        <div class="rotate_prompt_card">
            <div class="rotate_art" aria-hidden="true">
                <span class="rotate_arrow top"></span>
                <span class="rotate_phone"></span>
                <span class="rotate_arrow bottom"></span>
            </div>
            <h2>Landscape works best</h2>
            <p>Rotate your phone for a wider driving layout.</p>
        </div>
    </div>

    <div id="fullscreen_hint" class="fullscreen_hint" aria-live="polite">
        <div class="fullscreen_hint_card">
            <div class="tap_icon" aria-hidden="true">TAP</div>
            <h2>Tap for fullscreen</h2>
            <p>Chrome needs one touch before fullscreen can start.</p>
        </div>
    </div>

    <main>
        <div class="nav">
            <div class="nav_bar">
                <div class="p stat_box">
                    STATUS
                    <div class="stat_info">
                        <span id="stat_led"></span>
                        <span id="conn_stat">OFFLINE</span>
                    </div>
                </div>
                <div class="p ctrl_box">
                    STEER
                    <b id="str_angle_1">90</b>
                </div>
                <div class="p ctrl_box">
                    SPEED
                    <b id="speed_val_1">128</b>
                </div>
                <div class="p battery_box" aria-label="Battery telemetry">
                    BATTERY
                    <div class="battery_reading">
                        <span>Vbat</span>
                        <b id="vbat_val">--.-- V</b>
                    </div>
                    <div class="battery_reading">
                        <span>SoC</span>
                        <b id="soc_val">-- %</b>
                    </div>
                </div>
            </div>
            <div class="nav_bar top-tools">
                <button id="settings_btn" class="debug_btn">⚙ Settings</button>
                <div id="settings_panel" class="settings_overlay disp_none">
                    <button id="close_settings" class="close_settings_btn">
                        ❌
                    </button>
                    <div id="settings_home" class="settings_home">

                        <button class="settings_card" data-target="debug_section">
                            <span>Debug Panel</span>
                        </button>

                        <button class="settings_card" data-target="controls_section">
                            <span>Controls HUD</span>
                        </button>

                        <button class="settings_card" data-target="hardware_section">
                            <span>Hardware Check</span>
                        </button>

                        <button class="settings_card" data-target="wifi_section">
                            <span>Wifi Settings</span>
                        </button>

                        <button class="settings_card" data-target="updates_section">
                            <span>Updates</span>
                        </button>
                    </div>
                    <div class="settings_pages">

                        <!-- HARDWARE -->
                        <div id="hardware_section" class="settings_page disp_none">

                            <button class="back_btn">
                                ← Back
                            </button>

                            <h2>Hardware Check</h2>

                            <div class="settings_group">
                                <button id="servo_test">
                                    Servo Sweep Test
                                </button>

                                <button id="motor_test">
                                    Motor Test
                                </button>

                                <button id="rear_test">
                                    Rear Brake Test
                                </button>
                            </div>
                        </div>

                        <!-- CONTROLS -->
                        <div id="controls_section" class="settings_page disp_none">

                            <button class="back_btn">
                                ← Back
                            </button>

                            <h2>Controls HUD</h2>

                            <div id="controls_normal_settings" class="settings_grid">
                                <label class="setting_item">
                                    <span>Speed <b id="cfg_speed_out">128</b></span>
                                    <input id="cfg_speed" type="range" min="0" max="255" value="128">
                                </label>
                                <label class="setting_item">
                                    <span>Steer Center <b id="cfg_center_out">90</b></span>
                                    <input id="cfg_center" type="range" min="0" max="180" value="90">
                                </label>
                                <label class="setting_item">
                                    <span>Steer Min <b id="cfg_stmin_out">30</b></span>
                                    <input id="cfg_stmin" type="range" min="0" max="180" value="30">
                                </label>
                                <label class="setting_item">
                                    <span>Steer Max <b id="cfg_stmax_out">170</b></span>
                                    <input id="cfg_stmax" type="range" min="0" max="180" value="170">
                                </label>
                                <label class="setting_item">
                                    Motor Driver
                                    <select id="cfg_motor_driver">
                                        <option value="0">DIY Motor Driver</option>
                                        <option value="1">Commercial Motor Driver</option>
                                    </select>
                                </label>
                            </div>
                            <div id="controls_advanced_settings" class="disp_none">
                                <label class="setting_item">
                                    <span>Servo Min us <b id="cfg_servo_min_out">1000</b></span>
                                    <input id="cfg_servo_min" type="range" min="500" max="2500" value="1000">
                                </label>
                                <label class="setting_item">
                                    <span>Servo Max us <b id="cfg_servo_max_out">2000</b></span>
                                    <input id="cfg_servo_max" type="range" min="500" max="2500" value="2000">
                                </label>
                                <label class="setting_item">
                                    <span>Jump Value <b id="cfg_jsval_out">130</b></span>
                                    <input id="cfg_jsval" type="range" min="0" max="255" value="130">
                                </label>
                                <label class="setting_item">
                                    <span>Jump Duty <b id="cfg_jsduty_out">200</b></span>
                                    <input id="cfg_jsduty" type="range" min="0" max="255" value="200">
                                </label>
                                <label class="setting_item">
                                    <span>Jump Time ms <b id="cfg_jstime_out">50</b></span>
                                    <input id="cfg_jstime" type="range" min="0" max="500" value="50">
                                </label>
                            </div>
                            <div class="settings_actions">
                                <button id="controls_advanced_btn">Advanced</button>
                                <button id="toggle_steering_btn">Steering: NORMAL</button>
                                <button id="cfg_load_btn">Load Saved</button>
                                <button id="cfg_factory_btn">Factory Reset</button>
                            </div>
                        </div>

                        <!-- WIFI -->
                        <div id="wifi_section" class="settings_page disp_none">

                            <button class="back_btn">
                                ← Back
                            </button>

                            <h2>Wifi Settings</h2>

                            <div class="settings_grid">
                                <label class="setting_item">
                                    SSID
                                    <input id="cfg_ssid" type="text" maxlength="31" value="192.168.4.1">
                                </label>
                                <label class="setting_item">
                                    Password
                                    <input id="cfg_pass" type="password" minlength="8" maxlength="63" value="JD232323">
                                </label>
                            </div>
                            <p class="settings_note">WiFi name and password are saved to NVS only after Save. They apply
                                after the ESP32 reboots.</p>
                        </div>

                        <!-- UPDATE -->
                        <div id="updates_section" class="settings_page disp_none">

                            <button class="back_btn">
                                ← Back
                            </button>

                            <h2>System Updates</h2>

                            <button>
                                Check Firmware
                            </button>

                            <button>
                                Install Update
                            </button>
                        </div>

                    </div>
                    <div id="dirty_banner" class="dirty_banner disp_none">
                        <span>Unsaved settings</span>
                        <button id="cfg_save_btn" class="save_dirty_btn">Save</button>
                    </div>
                </div>
                <!-- Debug Panel -->
                <!-- <button id="debug_btn" class="debug_btn">Debug</button> -->
                <div id="db_panel" class="disp_none">
                    <i><span>Bus</span><span id="debug_conn_stat">READY</span>
                        <button id="close_debug_btn" class="">
                            ✕
                        </button>
                    </i>
                    <p id="debug_last_cmd">Idle</p>
                    <pre id="debug_logs">Idle</pre>
                </div>
            </div>
        </div>

        <div class="ctrl_sec">
            <div class="left_ctrls">
                <div class="p v">
                    <div class="u">
                        SPEED
                        <b id="p2">128</b>
                    </div>
                    <div class="vw">
                        <input id="speed_bar" type="range" min="0" max="255" value="128">
                    </div>
                </div>

                <div class="p w">
                    <div class="steer_reverse">
                        <input id="steer_bar" class="steer_bar" type="range" min="30" max="170" value="90">
                    </div>
                    <div class="u">
                        <b id="str_angle_2">90</b>
                        STEER
                    </div>
                </div>
            </div>

            <div class="right_ctrls">
                <div class="c">
                    <button id="ct" class="a">
                        <span>C</span>
                        <small>center</small>
                    </button>
                    <button id="rc" class="a">
                        <span>R</span>
                        <small>link</small>
                    </button>
                </div>

                <div class="ps">
                    <button id="br" class="y red"><span>BREAK</span></button>
                    <div class="pc">
                        <button id="fw" class="y"><span>RACE</span></button>
                        <button id="bk" class="y red"><span>BACK</span></button>
                    </div>
                </div>
            </div>
        </div>
    </main>

    <script>
        let w, t, P, R, S, D, C, L, P2, S2, pv, sv;
        let steeringReversed = false;       // Flag : Steering Direction
        let steerFlushTimer = null;
        let queuedSteerValue = 90;
        let latestSteerValue = -1;
        let lastFullscreenTry = 0;
        let activeHardwareTest = null;
        const K = {};
        const $ = (id) => document.getElementById(id);
        const cfgKeys = ["speed", "center", "stmin", "stmax", "motor_driver", "servo_min", "servo_max", "jsval", "jsduty", "jstime", "reverse", "ssid", "pass"];
        const cfgMap = {
            speed: "cfg_speed",
            center: "cfg_center",
            stmin: "cfg_stmin",
            stmax: "cfg_stmax",
            motor_driver: "cfg_motor_driver",
            servo_min: "cfg_servo_min",
            servo_max: "cfg_servo_max",
            jsval: "cfg_jsval",
            jsduty: "cfg_jsduty",
            jstime: "cfg_jstime",
            ssid: "cfg_ssid",
            pass: "cfg_pass"
        };
        const cfgSaved = {
            speed: 128,
            center: 90,
            stmin: 30,
            stmax: 170,
            motor_driver: 0,
            servo_min: 1000,
            servo_max: 2000,
            jsval: 130,
            jsduty: 200,
            jstime: 50,
            reverse: 0,
            ssid: "192.168.4.1",
            pass: "JD232323"
        };
        const cfgDraft = { ...cfgSaved };

        const updateBatteryTelemetry = (payload) => {
            const [voltage, soc] = payload.split(",").map(Number);
            if (Number.isFinite(voltage)) {
                $("vbat_val").textContent = voltage.toFixed(2) + " V";
            }
            if (Number.isFinite(soc)) {
                $("soc_val").textContent = Math.round(soc) + " %";
            }
        };

        const updateAppHeight = () => {
            const viewportHeight = window.visualViewport ? window.visualViewport.height : window.innerHeight;
            const viewportWidth = window.visualViewport ? window.visualViewport.width : window.innerWidth;
            document.documentElement.style.setProperty("--app-height", viewportHeight + "px");
            document.documentElement.style.setProperty("--app-width", viewportWidth + "px");
        };

        const updateFullscreenHint = () => {
            const hint = $("fullscreen_hint");
            if (!hint) {
                return;
            }
            const isLandscape = window.matchMedia("(orientation: landscape)").matches;
            const canFullscreen = !!document.documentElement.requestFullscreen;
            hint.classList.toggle("show", canFullscreen && isLandscape && !document.fullscreenElement);
        };

        const tryFullscreen = async () => {
            if (document.fullscreenElement || !document.documentElement.requestFullscreen) {
                updateFullscreenHint();
                return;
            }

            const now = Date.now();
            if (now - lastFullscreenTry < 650) {
                return;
            }

            lastFullscreenTry = now;

            try {
                await document.documentElement.requestFullscreen({ navigationUI: "hide" });
                document.documentElement.classList.add("is-fullscreen");
                updateAppHeight();
            } catch (error) {
                H("fs>browser");
            } finally {
                updateFullscreenHint();
            }
        };
   
        // ------------------ Steering Inversion Logic Start ------------------
        const getSteerAngle = (direction) => {
            if (!steeringReversed) {
                return direction === "left" ? cfgDraft.stmin : cfgDraft.stmax;
            }
            return direction === "left" ? cfgDraft.stmax : cfgDraft.stmin;
        };
      
        // ------------------ Steering Inversion Logic End --------------------

        const A = (element, active) => element.classList[active ? "add" : "remove"]("on");

        const U = (status, color) => {
            S.textContent = status;
            D.style.background = color;
            D.style.boxShadow = "0 0 8px " + color;
            $("debug_conn_stat").textContent = status;
        };

        const H = (text) => {
            const logs = L.textContent.split("\n");
            logs.push(text);
            L.textContent = logs.slice(-7).join("\n");
            // L.textContent = (L.textContent + "\n" + text).split("\n").slice(-7).join("\n");
            L.scrollTop = L.scrollHeight;
        };

        const M = (command) => {
            if (w && w.readyState === 1) {
                w.send(command);
            }
            C.textContent = command;
            H("tx>" + command);
        };

        const cfgValue = (key) => key === "ssid" || key === "pass" ? String(cfgDraft[key]).replace(/[\r\n,]/g, "") : Number(cfgDraft[key]);
        const liveCfgKeys = new Set(["speed", "center", "stmin", "stmax", "motor_driver", "servo_min", "servo_max", "jsval", "jsduty", "jstime", "reverse"]);

        const cfgDirty = () => cfgKeys.some(key => String(cfgDraft[key]) !== String(cfgSaved[key]));

        const setDirtyBanner = () => {
            $("dirty_banner").classList.toggle("disp_none", !cfgDirty());
        };

        const setCfgControl = (key) => {
            const id = cfgMap[key];
            if (!id) {
                return;
            }
            const input = $(id);
            if (!input) {
                return;
            }
            input.value = cfgDraft[key];
            const out = $(id + "_out");
            if (out) {
                out.textContent = cfgDraft[key];
            }
        };

        const applySteerReverseUi = () => {
            steeringReversed = !!Number(cfgDraft.reverse);
            $("toggle_steering_btn").textContent = "Steering: " + (steeringReversed ? "REVERSED" : "NORMAL");
            R.parentElement.classList.toggle("steer_reverse", steeringReversed);
        };

        const applyConfigUi = () => {
            cfgKeys.forEach(setCfgControl);
            P.value = cfgDraft.speed;
            pv.textContent = cfgDraft.speed;
            P2.textContent = cfgDraft.speed;
            R.min = cfgDraft.stmin;
            R.max = cfgDraft.stmax;
            queueSteer(cfgDraft.center, true);
            applySteerReverseUi();
            setDirtyBanner();
        };

        const sendLiveConfig = (key) => {
            if (liveCfgKeys.has(key)) {
                M("cfg_" + key + ":" + cfgValue(key));
            }
        };

        const markCfg = (key, value, live = true) => {
            cfgDraft[key] = key === "ssid" || key === "pass" ? String(value).replace(/[\r\n,]/g, "") : Number(value);
            setCfgControl(key);
            P.value = cfgDraft.speed;
            pv.textContent = cfgDraft.speed;
            P2.textContent = cfgDraft.speed;
            R.min = cfgDraft.stmin;
            R.max = cfgDraft.stmax;
            if (key === "reverse") {
                applySteerReverseUi();
            }
            if (key === "center" || key === "stmin" || key === "stmax") {
                queueSteer(cfgDraft.center, true);
            }
            if (live) {
                sendLiveConfig(key);
            }
            setDirtyBanner();
        };

        const syncConfig = (payload) => {
            const parts = payload.split(",");
            cfgKeys.forEach((key, index) => {
                if (parts[index] !== undefined) {
                    cfgSaved[key] = key === "ssid" || key === "pass" ? parts[index] : Number(parts[index]);
                    cfgDraft[key] = cfgSaved[key];
                }
            });
            applyConfigUi();
        };

        const saveConfig = () => {
            if (!w || w.readyState !== 1) {
                alert("Controller is offline. Reconnect before saving settings.");
                return;
            }
            const ssid = cfgValue("ssid");
            const pass = cfgValue("pass");
            if (!ssid || ssid.length > 31) {
                alert("SSID must be 1-31 characters.");
                return;
            }
            if (pass.length && (pass.length < 8 || pass.length > 63)) {
                alert("WiFi password must be 8-63 characters, or blank for open AP.");
                return;
            }
            if (cfgValue("stmin") > cfgValue("center") || cfgValue("center") > cfgValue("stmax")) {
                alert("Steer Center must be between Steer Min and Steer Max.");
                return;
            }
            cfgKeys.forEach(key => {
                const next = cfgValue(key);
                if (String(next) !== String(cfgSaved[key])) {
                    cfgSaved[key] = next;
                    cfgDraft[key] = next;
                }
            });
            M("cfg_save");
            applyConfigUi();
        };

        const flushSteer = () => {
            steerFlushTimer = null;
            if (queuedSteerValue !== latestSteerValue) {
                latestSteerValue = queuedSteerValue;
                M("steer:" + latestSteerValue);
            }
        };

        const queueSteer = (value, force = false) => {
            queuedSteerValue = Number(value);
            R.value = queuedSteerValue;
            sv.textContent = queuedSteerValue;
            S2.textContent = queuedSteerValue;

            if (force) {
                if (steerFlushTimer) {
                    clearTimeout(steerFlushTimer);
                    steerFlushTimer = null;
                }
                flushSteer();
                return;
            }

            if (!steerFlushTimer) {
                steerFlushTimer = setTimeout(flushSteer, 25);
            }
        };

        const finishHardwareTest = (test) => {
            if (!test || activeHardwareTest !== test) {
                return;
            }
            if (test.timer) {
                clearTimeout(test.timer);
            }
            test.cleanup();
            test.button.disabled = false;
            A(test.button, false);
            activeHardwareTest = null;
        };

        const runHardwareTest = (buttonId, duration, interval, tick, cleanup) => {
            const button = $(buttonId);
            if (activeHardwareTest) {
                if (activeHardwareTest.button === button) {
                    return;
                }
                finishHardwareTest(activeHardwareTest);
            }

            const test = { button, cleanup, timer: null };
            const startedAt = Date.now();
            activeHardwareTest = test;
            button.disabled = true;
            A(button, true);

            const step = () => {
                if (activeHardwareTest !== test) {
                    return;
                }
                const elapsed = Date.now() - startedAt;
                if (elapsed >= duration) {
                    finishHardwareTest(test);
                    return;
                }
                tick(elapsed);
                test.timer = setTimeout(step, interval);
            };

            step();
        };

        const X = () => {
            if (t) {
                clearTimeout(t);
            }

            w = new WebSocket("ws://" + location.host + "/ws");
            // w = new WebSocket("ws://" + "192.168.4.1" + "/ws");
            U("CONNECTING", "#ffd55a");
            H("ws>open");

            w.onopen = () => {
                U("CONNECTED", "#2cff95");
                H("ws>ok");
                M("hello");
                M("speed:" + P.value);
                latestSteerValue = -1;
                queueSteer(cfgDraft.center, true);
            };

            w.onmessage = (event) => {
                U(event.data || "CONNECTED", "#2cff95");
                H("rx>" + event.data);
                if (event.data && event.data.startsWith("cfg_sync:")) {
                    syncConfig(event.data.slice(9));
                } else if (event.data && event.data.startsWith("telemetry:")) {
                    updateBatteryTelemetry(event.data.slice(10));
                }
            };

            w.onclose = () => {
                U("OFFLINE", "#ff4f62");
                H("ws>off");
                t = setTimeout(X, 1000);
            };
        };

        const V = (value) => {
            P.value = value;
            pv.textContent = value;
            P2.textContent = value;
            M("speed:" + value);
        };

        const B = (id, startCommand, stopCommand) => {
            const element = $(id);

            const press = (event) => {
                event.preventDefault();
                element.setPointerCapture(event.pointerId); // Added Later
                tryFullscreen();
                A(element, true);
                M(startCommand);
            };

            const release = (event) => {
                event.preventDefault();
                element.releasePointerCapture(event.pointerId); // Added Later
                A(element, false);
                M(stopCommand);
            };

            element.onpointerdown = press;
            element.onpointerup = release;
            element.onpointercancel = release;
            element.onpointerleave = (event) => {
                if (event.buttons) {
                    release(event);
                }
            };
        };

        const Y = (key, down) => {
            if (down && K[key]) {
                return;
            }

            K[key] = down ? 1 : 0;

            if (key === "w" || key === "ArrowUp") {
                M(down ? "forward_start" : "forward_stop");
            } else if (key === "s" || key === "ArrowDown") {
                M(down ? "back_start" : "back_stop");
            } else if (key === "a" || key === "ArrowLeft") {
                queueSteer(down ? getSteerAngle("left") : cfgDraft.center, true);
            } else if (key === "d" || key === "ArrowRight") {
                queueSteer(down ? getSteerAngle("right") : cfgDraft.center, true);
            } else if (key === " ") {
                M(down ? "rear_break" : "break_stop");

            }
        };

        onload = () => {
            P = $("speed_bar");
            R = $("steer_bar");
            S = $("conn_stat");
            D = $("stat_led");
            C = $("debug_last_cmd");
            L = $("debug_logs");
            P2 = $("p2");
            S2 = $("str_angle_2");
            pv = $("speed_val_1");
            sv = $("str_angle_1");
            const steeringToggleBtn = $("toggle_steering_btn");
            const controlsAdvancedBtn = $("controls_advanced_btn");
            let controlsAdvancedMode = false;

            steeringToggleBtn.onclick = () => {
                markCfg("reverse", cfgDraft.reverse ? 0 : 1);
                H("steer>" + (steeringReversed ? "reversed" : "normal"));
            };

            controlsAdvancedBtn.onclick = () => {
                controlsAdvancedMode = !controlsAdvancedMode;
                $("controls_normal_settings").classList.toggle("disp_none", controlsAdvancedMode);
                $("controls_normal_settings").classList.toggle("settings_grid");
                $("controls_advanced_settings").classList.toggle("disp_none", !controlsAdvancedMode);
                $("controls_advanced_settings").classList.toggle("settings_grid");
                controlsAdvancedBtn.textContent = controlsAdvancedMode ? "Normal" : "Advanced";
            };

            updateAppHeight();
            updateFullscreenHint();
            window.addEventListener("resize", () => {
                updateAppHeight();
                updateFullscreenHint();
            });
            window.addEventListener("orientationchange", () => {
                setTimeout(() => {
                    updateAppHeight();
                    updateFullscreenHint();
                }, 250);
            });
            document.addEventListener("fullscreenchange", () => {
                document.documentElement.classList.toggle("is-fullscreen", !!document.fullscreenElement);
                updateAppHeight();
                updateFullscreenHint();
            });
            if (window.visualViewport) {
                window.visualViewport.addEventListener("resize", () => {
                    updateAppHeight();
                    updateFullscreenHint();
                });
                window.visualViewport.addEventListener("scroll", () => {
                    updateAppHeight();
                    updateFullscreenHint();
                });
            }

            P.oninput = () => {
                tryFullscreen();
                V(P.value);
            };

            R.oninput = () => {

                tryFullscreen();

                let value = Number(R.value);

                queueSteer(value);
            };

            // Auto-center steering when released
            R.onpointerup = () => {
                queueSteer(cfgDraft.center, true);
            };

            R.onpointercancel = () => {
                queueSteer(cfgDraft.center, true);
            };

            // Settings Panel Logic Start 

            const settingsPanel = $("settings_panel");
            const closeSettings = $("close_settings");
            const settingsBtn = $("settings_btn");
            const settingPages = document.querySelectorAll(".settings_page");
            const settingsHome = $("settings_home");

            // OPEN
            settingsBtn.onclick = () => {

                settingsPanel.classList.add("show");

                // show home
                settingsHome.classList.remove("disp_none");

                // hide pages
                settingPages.forEach(page => {
                    page.classList.add("disp_none");
                });
            };
            // CLOSE
            closeSettings.onclick = () => {
                settingsPanel.classList.remove("show");
            };
            $("close_debug_btn").onclick = () => {
                $("db_panel").classList.add("disp_none");
            }
            // OPEN INTERNAL PAGES
            // SETTINGS CARDS ACTIONS

            // OPEN PAGE
            document.querySelectorAll(".settings_card")
                .forEach(card => {

                    card.onclick = () => {

                        const targetId = card.dataset.target;

                        // DEBUG PANEL
                        if (targetId === "debug_section") {

                            settingsPanel.classList.remove("show");

                            $("db_panel").classList.remove("disp_none");

                            return;
                        }

                        const target = $(targetId);

                        // hide home
                        settingsHome.classList.add("disp_none");

                        // hide all pages
                        settingPages.forEach(page => {
                            page.classList.add("disp_none");
                        });

                        // show selected page
                        target.classList.remove("disp_none");
                    };
                });
            // BACK BUTTONS
            document.querySelectorAll(".back_btn")
                .forEach(btn => {

                    btn.onclick = () => {

                        // hide pages
                        settingPages.forEach(page => {
                            page.classList.add("disp_none");
                        });

                        // show home
                        settingsHome.classList.remove("disp_none");
                    };
                });

            Object.keys(cfgMap).forEach(key => {
                const input = $(cfgMap[key]);
                if (!input) {
                    return;
                }
                input.oninput = () => markCfg(key, input.value);
            });
            $("cfg_save_btn").onclick = saveConfig;
            $("cfg_load_btn").onclick = () => M("cfg_load");
            $("cfg_factory_btn").onclick = () => {
                if (confirm("Factory reset saved settings and reboot the ESP32?")) {
                    M("cfg_factory");
                }
            };
            $("servo_test").onclick = () => {
                runHardwareTest("servo_test", 4000, 60, (elapsed) => {
                    const phase = (elapsed % 1600) / 1600;
                    const min = Number(cfgDraft.stmin);
                    const max = Number(cfgDraft.stmax);
                    const angle = phase < .5 ? min + ((max - min) * phase * 2) : max - ((max - min) * (phase - .5) * 2);
                    queueSteer(Math.round(angle), true);
                }, () => {
                    queueSteer(cfgDraft.center, true);
                });
            };
            $("motor_test").onclick = () => {
                runHardwareTest("motor_test", 5000, 140, (elapsed) => {
                    const forward = elapsed < 2500;
                    const phaseElapsed = forward ? elapsed : elapsed - 2500;
                    const phase = (phaseElapsed % 1250) / 1250;
                    const duty = Math.round(90 + (140 * (phase < .5 ? phase * 2 : (1 - phase) * 2)));
                    M(forward ? "forward_start" : "back_start");
                    V(duty);
                }, () => {
                    M("forward_stop");
                    M("back_stop");
                    V(cfgDraft.speed);
                });
            };
            $("rear_test").onclick = () => {
                runHardwareTest("rear_test", 1600, 140, () => {
                    M("rear_break");
                }, () => {
                    M("break_stop");
                });
            };
            applyConfigUi();

            ct.onclick = () => queueSteer(cfgDraft.center, true);
            rc.onclick = () => (w && w.readyState < 2 ? w.close() : X());

            onkeydown = (event) => Y(event.key, true);
            onkeyup = (event) => Y(event.key, false);

            document.addEventListener("pointerdown", tryFullscreen);

            B("fw", "forward_start", "forward_stop");
            B("bk", "back_start", "back_stop");
            B("br", "rear_break", "break_stop");
            X();    // Uncomment This 
        };
    </script>
</body>

</html>

)rawliteral";

/* Macro Definitions Start */

#define GPIO_LED 2  // On board Blue LED

#define BATTERY_ADC_PIN 34  // ADC Vbat Measurement Pin

#define R_TOP 22000.0f
#define R_BOTTOM 15000.0f
#define DIVIDER_RATIO ((R_TOP + R_BOTTOM) / R_BOTTOM)

// ============================================================
// ADC Filter Configuration

// Number of samples in moving average
#define ADC_SAMPLES 64
// New sample must be within ±1% of filtered value
#define ADC_ACCEPTANCE_RATIO 0.01f
// ============================================================

// ============================================================
// Li-ion Battery SOC Lookup Table
// ============================================================
//
// This is a generic single-cell Li-ion discharge curve.
//
// Voltage      SOC
// ----------------
// 4.20 V       100%
// 4.15 V        95%
// 4.10 V        90%
// 4.05 V        85%
// 4.00 V        80%
// 3.95 V        75%
// 3.90 V        65%
// 3.85 V        55%
// 3.80 V        45%
// 3.75 V        35%
// 3.70 V        28%
// 3.65 V        20%
// 3.60 V        12%
// 3.55 V         5%
// 3.50 V         0%
//
// NOTE:
// Actual Li-ion SOC depends on chemistry, load,
// temperature and rest state. This table is intended
// as a practical BMS-style approximation.
//
// ============================================================

/****** Pinout for DIY Motor Driver ******/
#define Q1_PIN 19  // Q1 :- BD140 (PNP) transistor for backward direction
#define Q2_PIN 18  // Q2 :- BD140 (PNP) transistor for backward direction
#define Q3_PIN 5   // Q3 :- BD140 (PNP) transistor for backward direction
#define Q4_PIN 21  // Q4 :- BD140 (PNP) transistor for backward direction

/****** Pinout for L298N Motor Driver ******/
#define REAR_IN1_PIN 19  // IN1 Pin of Motor Driver
#define REAR_IN2_PIN 18  // IN2 Pin of Motor Driver
#define REAR_ENA_PIN 5   // ENA Pin of Motor Driver

#define SERVO_PIN 26  // Servo PWM Pin

#define PWM_FREQUENCY 50000  // 50k is best 20k frequency is noise effective & 30k frequency give low rpm tourqe
#define PWM_RESOLUTION 8     // 8-bit -> 256 Duty cycle steps

#define Steer_Min_Angle 30      // Min angle of Steering
#define Steer_Max_Angle 170     // Max angle of Steering
#define Steer_Staight_Angle 90  // Center of Steering

/* Macro Definition End */

/* Global Variables Start */

Preferences prefs;  // Global Object for storing values
struct CarConfig_t {
  int steerMin;
  int steerMax;
  int steerCenter;

  int servoMinUs;
  int servoMaxUs;

  int defaultSpeed;
  int motorDriver;

  int jumpStartValue;
  int jumpStartDuty;
  int jumpStartTimeout;

  bool steeringReversed;

  String ssid;
  String password;
};  // Structure to save all the values

CarConfig_t config;

struct SOC_Point {
  float voltage;
  float soc;
};


const SOC_Point socTable[] = {
  { 4.20f, 100.0f },
  { 4.15f, 95.0f },
  { 4.10f, 90.0f },
  { 4.05f, 85.0f },
  { 4.00f, 80.0f },
  { 3.95f, 75.0f },
  { 3.90f, 65.0f },
  { 3.85f, 55.0f },
  { 3.80f, 45.0f },
  { 3.75f, 35.0f },
  { 3.70f, 28.0f },
  { 3.65f, 20.0f },
  { 3.60f, 12.0f },
  { 3.55f, 5.0f },
  { 3.50f, 0.0f }
};


#define SOC_TABLE_SIZE \
  (sizeof(socTable) / sizeof(socTable[0]))


const char *ssid = "192.168.4.1";   // SSID of Wifi
const char *password = "12345678";  // Password of Wifi

const IPAddress apIP(192, 168, 4, 1);        // Static IP of ESP32 Itself
const IPAddress apGateway(192, 168, 4, 1);   // Static IP of Gateway (ap router)
const IPAddress apSubnet(255, 255, 255, 0);  //

AsyncWebServer server(80);  // Start Webserver on Port 80
AsyncWebSocket ws("/ws");
Servo servo;

// Rear Motor State enum
enum RearMotionMode {
  REAR_STOPPED,  // Steady State
  REAR_FORWARD,  // Forward Direction
  REAR_REVERSE,  // Reverse Direction
  REAR_BRAKING   // Braking
};

int minUs = 1000;            // 1000 micro second pulse width -> 0 degree angle
int maxUs = 2000;            // 2000 micro second pulse width -> 180 degree angle
int JumpStart_Value = 130;   // The Jumpstart apply level
int JumpStart_duty = 200;    // Start Jumstart feature from this duty cycle
int JumpStart_timeout = 50;  // the time for Jumpstart to apply
int speedValue = 128;        // Default Speed value
int targetSteerAngle = 90;   // Desired Steering angle
int appliedSteerAngle = 90;  // Current Steering angle

RearMotionMode rearMotion = REAR_STOPPED;  // Global enum for Current enum State
int rearDutyTarget = 0;
bool jumpActive = false;
unsigned long jumpEndAt = 0;

// constexpr uint16_t DNS_PORT = 53;
constexpr unsigned long STEER_UPDATE_INTERVAL_MS = 20;  // 20 ms Steering Update Time

bool H = LOW;
bool L = HIGH;

// ============================================================
// Moving Average Variables
// ============================================================

float voltageBuffer[ADC_SAMPLES];

float voltageSum = 0.0f;
uint32_t bufferIndex = 0;
uint32_t samplesUsed = 0;
bool filterInitialized = false;

/* Global Variables End */

/* Function Prototypes Start */

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void serveIndex(AsyncWebServerRequest *request);
size_t streamIndexHtml(uint8_t *buffer, size_t maxLen, size_t index);
void handleConfigCommand(const char *command);
void sendConfig(AsyncWebSocketClient *client);
String buildConfigMessage();


float updateADCFilter(float newVoltage);
float calculateBatteryVoltage(float adcVoltage);
float calculateSOC(float batteryVoltage);

/* Function Prototypes End */

/* Functions Definitions Start */

/* ------------------------- NVS Storage Functions ------------------ */
void loadDefaultConfig() {
  config.steerMin = 30;
  config.steerMax = 170;
  config.steerCenter = 90;

  config.servoMinUs = 1000;
  config.servoMaxUs = 2000;

  config.defaultSpeed = 128;
  config.motorDriver = 0;

  config.jumpStartValue = 130;
  config.jumpStartDuty = 200;
  config.jumpStartTimeout = 50;

  config.steeringReversed = false;

  config.ssid = "192.168.4.1";
  config.password = "JD232323";
}
void loadConfig() {
  prefs.begin("fpvcar", true);

  config.steerMin = prefs.getInt("stMin", 30);
  config.steerMax = prefs.getInt("stMax", 170);
  config.steerCenter = prefs.getInt("stCtr", 90);
  config.steeringReversed = prefs.getBool("rev", false);

  config.servoMinUs = prefs.getInt("sMinUs", 1000);
  config.servoMaxUs = prefs.getInt("sMaxUs", 2000);

  config.defaultSpeed = prefs.getInt("speed", 128);
  config.motorDriver = prefs.getInt("mDrv", 0);
  config.jumpStartValue = prefs.getInt("jsVal", 130);
  config.jumpStartDuty = prefs.getInt("jsDuty", 200);
  config.jumpStartTimeout = prefs.getInt("jsTime", 50);

  config.ssid = prefs.getString("ssid", "192.168.4.1");
  config.password = prefs.getString("pass", "JD232323");
  prefs.end();
}

void saveConfig() {
  prefs.begin("fpvcar", false);

  prefs.putInt("stMin", config.steerMin);
  prefs.putInt("stMax", config.steerMax);
  prefs.putInt("stCtr", config.steerCenter);
  prefs.putBool("rev", config.steeringReversed);

  prefs.putInt("sMinUs", config.servoMinUs);
  prefs.putInt("sMaxUs", config.servoMaxUs);

  prefs.putInt("speed", config.defaultSpeed);
  prefs.putInt("mDrv", config.motorDriver);

  prefs.putInt("jsVal", config.jumpStartValue);
  prefs.putInt("jsDuty", config.jumpStartDuty);
  prefs.putInt("jsTime", config.jumpStartTimeout);

  prefs.putString("ssid", config.ssid);
  prefs.putString("pass", config.password);

  prefs.end();
}


/*-------------- Hardware Driver for Motor Control ----------------*/

void writeRearOutputs(RearMotionMode mode, int duty_cycle) {
  duty_cycle = constrain(duty_cycle, 0, 255);  // Stop Invalid Inputs

  /* ==== Commercial Motor Driver Logic === */
  if (config.motorDriver == 1) {
    if (mode == REAR_FORWARD) {
      digitalWrite(REAR_IN1_PIN, HIGH);
      digitalWrite(REAR_IN2_PIN, LOW);
      ledcWrite(REAR_ENA_PIN, duty_cycle);
    } else if (mode == REAR_REVERSE) {
      digitalWrite(REAR_IN1_PIN, LOW);
      digitalWrite(REAR_IN2_PIN, HIGH);
      ledcWrite(REAR_ENA_PIN, duty_cycle);
    } else if (mode == REAR_BRAKING) {
      digitalWrite(REAR_IN1_PIN, HIGH);
      digitalWrite(REAR_IN2_PIN, HIGH);
      ledcWrite(REAR_ENA_PIN, 255);
    } else {
      digitalWrite(REAR_IN1_PIN, LOW);
      digitalWrite(REAR_IN2_PIN, LOW);
      ledcWrite(REAR_ENA_PIN, 0);
    }
    return;
  }

  /* === DIY Motoro Driver Logic === */
  if (mode == REAR_FORWARD) {
    digitalWrite(Q1_PIN, H);
    digitalWrite(Q2_PIN, L);
    ledcWrite(Q3_PIN, duty_cycle);
    ledcWrite(Q4_PIN, 255);
  } else if (mode == REAR_REVERSE) {
    digitalWrite(Q1_PIN, L);
    digitalWrite(Q2_PIN, H);
    ledcWrite(Q3_PIN, 255);
    ledcWrite(Q4_PIN, duty_cycle);
  } else if (mode == REAR_BRAKING) {
    digitalWrite(Q1_PIN, H);
    digitalWrite(Q2_PIN, H);
    ledcWrite(Q3_PIN, 0);
    ledcWrite(Q4_PIN, 0);
  } else {
    digitalWrite(Q1_PIN, H);
    digitalWrite(Q2_PIN, H);
    ledcWrite(Q3_PIN, 255);
    ledcWrite(Q4_PIN, 255);
  }
}

/*-------------- Rear Control Wrapper for Hardware Driver----------------*/
void applyRearState() {
  if (rearMotion == REAR_FORWARD || rearMotion == REAR_REVERSE) {
    int dutyToApply = rearDutyTarget;
    if (jumpActive && (millis() < jumpEndAt)) {
      dutyToApply = JumpStart_Value;  // Apply JumpStart Value Until Timeout
    } else {
      jumpActive = false;
    }
    writeRearOutputs(rearMotion, dutyToApply);
  } else {
    jumpActive = false;
    writeRearOutputs(rearMotion, 0);
  }
}

/*-------------- Jumpstar Wrapper for Forward and Reverse Rear ----------------*/
void startRearMotion(RearMotionMode mode, int duty_cycle) {
  rearMotion = mode;
  rearDutyTarget = constrain(duty_cycle, 0, 255);
  jumpActive = rearDutyTarget >= JumpStart_duty && rearDutyTarget < 255;  // Apply Jumpstart for the values above JumStart_duty
  jumpEndAt = millis() + JumpStart_timeout;                               //
  applyRearState();
}

// Forward Rear Function
void Forward_Rear(int duty_cycle) {
  startRearMotion(REAR_FORWARD, duty_cycle);
}

// Reverse Rear Function
void Reverse_Rear(int duty_cycle) {
  startRearMotion(REAR_REVERSE, duty_cycle);
}

// Rear Stop Function
void Rear_Stop() {
  rearMotion = REAR_STOPPED;
  applyRearState();
}

// Rear Break Function
void Rear_Break() {
  rearMotion = REAR_BRAKING;
  applyRearState();
}

/*---------------- Set Steering Target Function ---------------*/
void SetSteeringTarget(int angle) {
  targetSteerAngle = constrain(angle, config.steerMin, config.steerMax);
}

/* Immediately Change Steering Angle */
void SetSteeringAngleImmediate(int angle) {
  appliedSteerAngle = constrain(angle, config.steerMin, config.steerMax);
  targetSteerAngle = appliedSteerAngle;
  servo.write(appliedSteerAngle);
}

/* --------------- Absolute Steering Functions Start -----------------*/
void Set_straight() {
  SetSteeringTarget(config.steerCenter);
}

void Left_Turn() {
  SetSteeringTarget(config.steerMax);
}

void Right_Turn() {
  SetSteeringTarget(config.steerMin);
}

/* --------------- Absolute Steering Functions END -----------------*/

/* --------------- Hardware Check Function -----------------*/
void Hardware_check() {
  writeRearOutputs(REAR_FORWARD, 240);
  delay(200);
  writeRearOutputs(REAR_STOPPED, 0);
  delay(200);
  writeRearOutputs(REAR_REVERSE, 240);
  delay(200);
  writeRearOutputs(REAR_STOPPED, 0);
  SetSteeringAngleImmediate(config.steerMax);
  delay(100);
  SetSteeringAngleImmediate(config.steerCenter);
  delay(100);
  SetSteeringAngleImmediate(config.steerMin);
  delay(100);
  SetSteeringAngleImmediate(config.steerCenter);
}


/* ----------------- Hardware Loop Service function Start --------------- */
void serviceSteering() {
  const unsigned long now = millis();
  static unsigned long lastSteerUpdateAt = 0;

  if ((now - lastSteerUpdateAt) < STEER_UPDATE_INTERVAL_MS) {
    return;  // Return If called Before Timeout
  }

  lastSteerUpdateAt = now;  // Update

  if (appliedSteerAngle != targetSteerAngle) {
    appliedSteerAngle = targetSteerAngle;
    servo.write(appliedSteerAngle);
  }
}

// Function to Monitor Rear Jumpstart Feature
void serviceRearMotion() {
  if (!jumpActive) {
    return;  // Return if Jumpstart is off
  }
  if (millis() >= jumpEndAt) {
    jumpActive = false;
    applyRearState();
  }
}

// Function for Vbat Measurement and SoC calculations
void serviceVbatSoc(void) {
  static uint32_t u32Prev_ms_tick = 0;

  if (millis() - u32Prev_ms_tick > 100) {
    u32Prev_ms_tick = millis();

    int rawADC = analogRead(BATTERY_ADC_PIN);
    uint32_t adcVoltage_mV = analogReadMilliVolts(BATTERY_ADC_PIN);
    float adcVoltage = adcVoltage_mV / 1000.0f;
    float filteredADCVoltage = updateADCFilter(adcVoltage);
    float batteryVoltage = calculateBatteryVoltage(filteredADCVoltage);
    float batterySOC = calculateSOC(batteryVoltage);


    // =============== Telemetry of Vbat para ==================
    static uint32_t lastTelemetry = 0;

    if (millis() - lastTelemetry >= 200) {
        lastTelemetry = millis();

        String telemetry = "telemetry:";
        telemetry += String(batteryVoltage, 2);
        telemetry += ",";
        telemetry += String(batterySOC, 1);

        ws.textAll(telemetry);
    }

    // Serial.print("Raw ADC: ");
    // Serial.print(rawADC);

    // Serial.print(" | ADC: ");
    // Serial.print(filteredADCVoltage, 4);
    // Serial.print(" V");

    // Serial.print(" | Battery: ");
    // Serial.print(batteryVoltage, 3);
    // Serial.print(" V");

    // Serial.print(" | SOC: ");
    // Serial.print(batterySOC, 1);
    // Serial.println(" %");
  }
}
/* ----------------- Hardware Loop Service function End --------------- */

// ============================================================
// ADC Moving Average Filter
// ============================================================
//
// A sample is accepted only when it is within ±1% of the
// current filtered voltage.
//
// This prevents sudden ADC spikes from entering the moving
// average and disturbing the battery voltage measurement.
//
// ============================================================

float updateADCFilter(float newVoltage) {
  // --------------------------------------------------------
  // First sample initializes the filter
  // --------------------------------------------------------

  if (!filterInitialized) {
    for (uint32_t i = 0; i < ADC_SAMPLES; i++) {
      voltageBuffer[i] = newVoltage;
    }

    voltageSum = newVoltage * ADC_SAMPLES;

    bufferIndex = 0;

    samplesUsed = ADC_SAMPLES;

    filterInitialized = true;

    return newVoltage;
  }


  // --------------------------------------------------------
  // Calculate current filtered value
  // --------------------------------------------------------

  float filteredVoltage =
    voltageSum / samplesUsed;


  // --------------------------------------------------------
  // Calculate ±1% acceptance window
  // --------------------------------------------------------

  float acceptance =
    filteredVoltage * ADC_ACCEPTANCE_RATIO;


  float lowerLimit =
    filteredVoltage - acceptance;


  float upperLimit =
    filteredVoltage + acceptance;


  // --------------------------------------------------------
  // Reject sample if outside acceptance window
  // --------------------------------------------------------

  if ((newVoltage < lowerLimit) || (newVoltage > upperLimit)) {
    return filteredVoltage;
  }


  // --------------------------------------------------------
  // Remove oldest sample
  // --------------------------------------------------------

  voltageSum -= voltageBuffer[bufferIndex];


  // --------------------------------------------------------
  // Add new sample
  // --------------------------------------------------------

  voltageBuffer[bufferIndex] =
    newVoltage;

  voltageSum += newVoltage;


  // --------------------------------------------------------
  // Advance buffer index
  // --------------------------------------------------------

  bufferIndex++;

  if (bufferIndex >= ADC_SAMPLES) {
    bufferIndex = 0;
  }


  // --------------------------------------------------------
  // Return filtered result
  // --------------------------------------------------------

  return voltageSum / samplesUsed;
}


// ============================================================
// Battery Voltage Calculation
// ============================================================

float calculateBatteryVoltage(float adcVoltage) {
  return adcVoltage * DIVIDER_RATIO;
}


// ============================================================
// Li-ion SOC Calculation
// ============================================================
//
// Uses linear interpolation between lookup-table points.
//
// ============================================================

float calculateSOC(float batteryVoltage) {
  // --------------------------------------------------------
  // Above maximum voltage
  // --------------------------------------------------------

  if (batteryVoltage >= socTable[0].voltage) {
    return 100.0f;
  }


  // --------------------------------------------------------
  // Below minimum voltage
  // --------------------------------------------------------

  if (batteryVoltage <= socTable[SOC_TABLE_SIZE - 1].voltage) {
    return 0.0f;
  }


  // --------------------------------------------------------
  // Find voltage interval
  // --------------------------------------------------------

  for (uint32_t i = 0;
       i < SOC_TABLE_SIZE - 1;
       i++) {
    float upperVoltage = socTable[i].voltage;

    float lowerVoltage = socTable[i + 1].voltage;


    if ((batteryVoltage <= upperVoltage) && (batteryVoltage >= lowerVoltage)) {
      float upperSOC = socTable[i].soc;

      float lowerSOC = socTable[i + 1].soc;


      // ------------------------------------------------
      // Linear interpolation
      // ------------------------------------------------

      float ratio = (batteryVoltage - lowerVoltage) / (upperVoltage - lowerVoltage);


      float soc = lowerSOC + ratio * (upperSOC - lowerSOC);


      return soc;
    }
  }


  // Safety fallback
  return 0.0f;
}
/* Functions Definitions End */

/* Main Application Start */

void setup() {
  /*---------- Hardware Setup ------------- */
  //------ Initialize NVS settings Start --------

  loadDefaultConfig();
  loadConfig();
  speedValue = config.defaultSpeed;

  minUs = config.servoMinUs;
  maxUs = config.servoMaxUs;

  JumpStart_Value = config.jumpStartValue;
  JumpStart_duty = config.jumpStartDuty;
  JumpStart_timeout = config.jumpStartTimeout;

  //   ------ Initialize NVS settings End -------

  Serial.begin(115200);

  pinMode(GPIO_LED, OUTPUT);  // On Board Blue LED

  pinMode(Q1_PIN, OUTPUT);                            // Q1 Transistor
  pinMode(Q2_PIN, OUTPUT);                            // Q2 Transistor
  ledcAttach(Q3_PIN, PWM_FREQUENCY, PWM_RESOLUTION);  // Q3 Transistor
  ledcAttach(Q4_PIN, PWM_FREQUENCY, PWM_RESOLUTION);  // Q4 Transistor

  servo.setPeriodHertz(50);                       // 50 Hz Servo
  servo.attach(SERVO_PIN, minUs, maxUs);          // 1000 to 2000 micro-second pulse width
  SetSteeringAngleImmediate(config.steerCenter);  // Default Agnel to 90
  /*---------- Vbat ADC Setup ------------- */
  analogReadResolution(12);
  // Original ESP32
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
  for (uint32_t i = 0; i < ADC_SAMPLES; i++) {
    voltageBuffer[i] = 0.0f;
  }

  /*---------- Webserver Setup ------------- */
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  WiFi.softAP(config.ssid.c_str(), config.password.c_str());

  Serial.println("AP started");
  Serial.println(WiFi.softAPIP());

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, serveIndex);
  server.on("/index.html", HTTP_GET, serveIndex);
  server.on("/stream", HTTP_ANY, [](AsyncWebServerRequest *request) {
    request->send(204);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not Found Try 192.168.4.1");  // On Invalid Request Suggest actual IP
  });

  server.begin();

  /* Initial Hardware Check */
  Hardware_check();
  Rear_Stop();
  SetSteeringTarget(config.steerCenter);
}

/*-------------------------- Main App Infinite Loop -----------------------*/
void loop() {
  //   dnsServer.processNextRequest();
  ws.cleanupClients();
  serviceRearMotion();  // Monitors Rear Jumpstart
  serviceSteering();
  serviceVbatSoc();
}
/* Main Application End */

/* Callback Logic Start */

/* ----------------------------------------  Websocket Callback Function--------------------------------- */

/* Websocket Callback Function ( Lightweight String Processing ) */
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (!(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)) {
    return;
  }
  // Prevent overflow
  if (len >= 64) {
    return;
  }
  char command[64];
  memcpy(command, data, len);
  command[len] = '\0';
  // ---------------- NVS Settings Commands ----------------
  // cfg_speed:200
  // cfg_center:90
  // cfg_stmin:45
  // cfg_stmax:135
  // cfg_motor_driver:1
  // cfg_servo_min:500
  // cfg_servo_max:2500
  // cfg_jsval:150
  // cfg_jsduty:255
  // cfg_jstime:250
  // cfg_reverse:1
  // cfg_ssid:MyCar
  // cfg_pass:12345678
  // cfg_save
  // cfg_load
  // cfg_factory
  if (strncmp(command, "cfg_", 4) == 0) {
    handleConfigCommand(command);
    return;
  }

  // ---------------- Motion Commands ----------------
  if (strcmp(command, "forward_start") == 0) {
    Forward_Rear(speedValue);
  } else if (strcmp(command, "back_start") == 0) {
    Reverse_Rear(speedValue);
  } else if (strcmp(command, "forward_stop") == 0 || strcmp(command, "back_stop") == 0 || strcmp(command, "break_stop") == 0) {
    Rear_Stop();
  } else if (strcmp(command, "rear_break") == 0) {
    Rear_Break();
  }
  // ---------------- Steering Commands ----------------
  else if (strcmp(command, "left_start") == 0) {
    Left_Turn();
  } else if (strcmp(command, "right_start") == 0) {
    Right_Turn();
  } else if (strcmp(command, "left_stop") == 0 || strcmp(command, "right_stop") == 0) {
    Set_straight();
  }

  // ---------------- Speed Command ----------------
  else if (strncmp(command, "speed:", 6) == 0) {
    speedValue = constrain(atoi(command + 6), 0, 255);
    rearDutyTarget = speedValue;
    if (rearMotion == REAR_FORWARD || rearMotion == REAR_REVERSE) {
      applyRearState();
    }
  }

  // ---------------- Steering Angle Command ----------------
  else if (strncmp(command, "steer:", 6) == 0) { SetSteeringTarget(atoi(command + 6)); }

  // ---------------- Handshake ----------------
  else if (strcmp(command, "hello") == 0) {
    ws.textAll("CONNECTED");
  }
}

/* WebServer Event Handler */
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket client #%u connected\n", client->id());
    client->text("CONNECTED");
    sendConfig(client);
    digitalWrite(GPIO_LED, HIGH);
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    digitalWrite(GPIO_LED, LOW);
  } else if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

/* Send WebPage HTML handler Function */
void serveIndex(AsyncWebServerRequest *request) {
  // The controller is now larger than 64 KB.  Stream it instead of using a
  // single PROGMEM response, which can be truncated by some AsyncWebServer
  // versions and leaves the browser waiting for a page that never completes.
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html; charset=utf-8", streamIndexHtml);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

size_t streamIndexHtml(uint8_t *buffer, size_t maxLen, size_t index) {
  const size_t htmlLength = strlen_P(index_html);
  if (index >= htmlLength) {
    return 0;
  }

  const size_t remaining = htmlLength - index;
  const size_t bytesToCopy = remaining < maxLen ? remaining : maxLen;
  memcpy_P(buffer, index_html + index, bytesToCopy);
  return bytesToCopy;
}

/* NVS Configuration Handler Function */
void handleConfigCommand(const char *command) {
  // ---------------- Speed ----------------
  if (strncmp(command, "cfg_speed:", 10) == 0) {
    config.defaultSpeed =
      constrain(atoi(command + 10), 0, 255);

    speedValue = config.defaultSpeed;

    ws.textAll("cfg_speed_ok");
  }

  // ---------------- Steering Center ----------------
  else if (strncmp(command, "cfg_center:", 11) == 0) {
    config.steerCenter =
      constrain(atoi(command + 11), 0, 180);

    ws.textAll("cfg_center_ok");
  }

  // ---------------- Steering Min ----------------
  else if (strncmp(command, "cfg_stmin:", 10) == 0) {
    config.steerMin =
      constrain(atoi(command + 10), 0, 180);

    ws.textAll("cfg_stmin_ok");
  }

  // ---------------- Steering Max ----------------
  else if (strncmp(command, "cfg_stmax:", 10) == 0) {
    config.steerMax =
      constrain(atoi(command + 10), 0, 180);

    ws.textAll("cfg_stmax_ok");
  }

  // ---------------- Motor Driver ----------------
  else if (strncmp(command, "cfg_motor_driver:", 17) == 0) {
    config.motorDriver =
      constrain(atoi(command + 17), 0, 1);

    Rear_Stop();

    ws.textAll("cfg_motor_driver_ok");
  }

  // ---------------- Servo Min Pulse ----------------
  else if (strncmp(command, "cfg_servo_min:", 14) == 0) {
    config.servoMinUs =
      constrain(atoi(command + 14), 500, 2500);

    minUs = config.servoMinUs;
    servo.attach(SERVO_PIN, minUs, maxUs);

    ws.textAll("cfg_servo_min_ok");
  }

  // ---------------- Servo Max Pulse ----------------
  else if (strncmp(command, "cfg_servo_max:", 14) == 0) {
    config.servoMaxUs =
      constrain(atoi(command + 14), 500, 2500);

    maxUs = config.servoMaxUs;
    servo.attach(SERVO_PIN, minUs, maxUs);

    ws.textAll("cfg_servo_max_ok");
  }

  // ---------------- JumpStart Value ----------------
  else if (strncmp(command, "cfg_jsval:", 10) == 0) {
    config.jumpStartValue =
      constrain(atoi(command + 10), 0, 255);

    JumpStart_Value = config.jumpStartValue;

    ws.textAll("cfg_jsval_ok");
  }

  // ---------------- JumpStart Duty ----------------
  else if (strncmp(command, "cfg_jsduty:", 11) == 0) {
    config.jumpStartDuty =
      constrain(atoi(command + 11), 0, 255);

    JumpStart_duty = config.jumpStartDuty;

    ws.textAll("cfg_jsduty_ok");
  }

  // ---------------- JumpStart Timeout ----------------
  else if (strncmp(command, "cfg_jstime:", 11) == 0) {
    config.jumpStartTimeout =
      constrain(atoi(command + 11), 0, 500);

    JumpStart_timeout = config.jumpStartTimeout;

    ws.textAll("cfg_jstime_ok");
  }

  // ---------------- Steering Reverse ----------------
  else if (strncmp(command, "cfg_reverse:", 12) == 0) {
    config.steeringReversed =
      atoi(command + 12);

    ws.textAll("cfg_reverse_ok");
  }

  // ---------------- Wifi SSID ----------------
  else if (strncmp(command, "cfg_ssid:", 9) == 0) {
    String nextSsid = String(command + 9);
    nextSsid.trim();

    if (nextSsid.length() > 0 && nextSsid.length() <= 31) {
      config.ssid = nextSsid;
      ws.textAll("cfg_ssid_ok");
    } else {
      ws.textAll("cfg_ssid_bad");
    }
  }

  // ---------------- Wifi Password ----------------
  else if (strncmp(command, "cfg_pass:", 9) == 0) {
    String nextPass = String(command + 9);

    if (nextPass.length() == 0 || (nextPass.length() >= 8 && nextPass.length() <= 63)) {
      config.password = nextPass;
      ws.textAll("cfg_pass_ok");
    } else {
      ws.textAll("cfg_pass_bad");
    }
  }

  // ---------------- Save ----------------
  else if (strcmp(command, "cfg_save") == 0) {
    saveConfig();

    ws.textAll("cfg_saved");
  }

  // ---------------- Reload ----------------
  else if (strcmp(command, "cfg_load") == 0) {
    loadConfig();

    speedValue = config.defaultSpeed;
    config.motorDriver = constrain(config.motorDriver, 0, 1);
    minUs = config.servoMinUs;
    maxUs = config.servoMaxUs;
    JumpStart_Value = config.jumpStartValue;
    JumpStart_duty = config.jumpStartDuty;
    JumpStart_timeout = config.jumpStartTimeout;
    servo.attach(SERVO_PIN, minUs, maxUs);
    SetSteeringTarget(config.steerCenter);

    ws.textAll("cfg_loaded");
    ws.textAll(buildConfigMessage());
  }

  // ---------------- Factory Reset ----------------
  else if (strcmp(command, "cfg_factory") == 0) {
    prefs.begin("fpvcar", false);
    prefs.clear();
    prefs.end();

    ws.textAll("factory_reset");

    delay(500);

    ESP.restart();
  }
}

String buildConfigMessage() {
  String msg;

  msg += "cfg_sync:";
  msg += String(config.defaultSpeed);
  msg += ",";
  msg += String(config.steerCenter);
  msg += ",";
  msg += String(config.steerMin);
  msg += ",";
  msg += String(config.steerMax);
  msg += ",";
  msg += String(config.motorDriver);
  msg += ",";
  msg += String(config.servoMinUs);
  msg += ",";
  msg += String(config.servoMaxUs);
  msg += ",";
  msg += String(config.jumpStartValue);
  msg += ",";
  msg += String(config.jumpStartDuty);
  msg += ",";
  msg += String(config.jumpStartTimeout);
  msg += ",";
  msg += String(config.steeringReversed);
  msg += ",";
  msg += config.ssid;
  msg += ",";
  msg += config.password;

  return msg;
}

/* Function to Send Websocket Configs */
void sendConfig(AsyncWebSocketClient *client) {
  String msg = buildConfigMessage();

  client->text(msg);
}
/* Callback Logic END */
