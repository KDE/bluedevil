/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

import org.kde.bluezqt as BluezQt

ColumnLayout {
    id: mediaPlayer

    spacing: 0

    PlasmaComponents3.Label {
        id: trackTitleLabel
        Layout.fillWidth: true
        elide: Text.ElideRight
        font.weight: MediaPlayer && MediaPlayer.track.title ? Font.DemiBold : Font.Normal
        font.italic: MediaPlayer && MediaPlayer.status === BluezQt.MediaPlayer.Playing
        font.pointSize: Kirigami.Theme.smallFont.pointSize
        font.family: Kirigami.Theme.smallFont.family
        opacity: 0.6
        text: trackTitleText()
        textFormat: Text.PlainText
        visible: text.length
    }

    PlasmaComponents3.Label {
        id: trackArtistLabel
        Layout.fillWidth: true
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
        opacity: 0.6
        text: MediaPlayer ? MediaPlayer.track.artist : ""
        textFormat: Text.PlainText
        visible: text.length
    }

    PlasmaComponents3.Label {
        id: trackAlbumLabel
        Layout.fillWidth: true
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
        opacity: 0.6
        text: MediaPlayer ? MediaPlayer.track.album : ""
        textFormat: Text.PlainText
        visible: text.length
    }

    RowLayout {
        spacing: 0

        PlasmaComponents3.ToolButton {
            id: previousButton
            icon.name: "media-skip-backward-symbolic"

            onClicked: MediaPlayer.previous()
        }

        PlasmaComponents3.ToolButton {
            id: playPauseButton
            icon.name: playPauseButtonIcon()

            onClicked: playPauseButtonClicked()
        }

        PlasmaComponents3.ToolButton {
            id: stopButton
            icon.name: "media-playback-stop-symbolic"
            enabled: MediaPlayer && MediaPlayer.status !== BluezQt.MediaPlayer.Stopped

            onClicked: MediaPlayer.stop()
        }

        PlasmaComponents3.ToolButton {
            id: nextButton
            icon.name: "media-skip-forward-symbolic"

            onClicked: MediaPlayer.next()
        }
    }

    function trackTitleText() {
        if (!MediaPlayer) {
            return "";
        }

        const play = "\u25B6";

        if (MediaPlayer.status === BluezQt.MediaPlayer.Playing) {
            return "%1 %2".arg(play).arg(MediaPlayer.track.title);
        }
        return MediaPlayer.track.title;
    }

    function playPauseButtonIcon() {
        if (!MediaPlayer) {
            return "";
        }

        if (MediaPlayer.status !== BluezQt.MediaPlayer.Playing) {
            return "media-playback-start-symbolic";
        } else {
            return "media-playback-pause-symbolic";
        }
    }

    function playPauseButtonClicked() {
        if (MediaPlayer.status !== BluezQt.MediaPlayer.Playing) {
            MediaPlayer.play()
        } else {
            MediaPlayer.pause()
        }
    }
}
