import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    id: page
    spacing: 18
    property bool listMode: false
    property var contextClip: ({})
    function applyFilter() {
        clipLibrary.filter(search.text, sort.model[sort.currentIndex].value, onlyFav.checked);
    }
    // Typing triggers one SQL query after a short pause instead of one per keystroke.
    Timer {
        id: searchDelay
        interval: 180
        onTriggered: page.applyFilter()
    }
    Shortcut {
        sequence: StandardKey.Find
        onActivated: {
            search.forceActiveFocus();
            search.selectAll();
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Mes clips"
            color: Theme.text
            font.pixelSize: 28
            font.weight: Font.DemiBold
        }
        Text {
            text: clipLibrary.count
            color: Theme.secondary
            font.pixelSize: 18
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Importer"
            onClicked: appController.importVideos()
        }
        ActionButton {
            text: "Actualiser"
            onClicked: appController.refresh()
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        CField {
            id: search
            Layout.fillWidth: true
            placeholderText: "Rechercher un nom, un jeu ou un tag  (Ctrl+F)"
            onTextChanged: searchDelay.restart()
            Keys.onEscapePressed: text = ""
        }
        CCombo {
            id: sort
            textRole: "text"
            model: [
                {
                    text: "Date",
                    value: "created"
                },
                {
                    text: "Durée",
                    value: "duration"
                },
                {
                    text: "Taille",
                    value: "size"
                },
                {
                    text: "Nom",
                    value: "name"
                }
            ]
            onActivated: page.applyFilter()
        }
        CheckBox {
            id: onlyFav
            text: "Favoris"
            onToggled: page.applyFilter()
        }
        ActionButton {
            text: page.listMode ? "Grille" : "Liste"
            onClicked: page.listMode = !page.listMode
        }
    }
    Text {
        visible: clipLibrary.count === 0
        text: search.text.length > 0 || onlyFav.checked ? "Aucun clip ne correspond à ces filtres." : "Aucun clip pour l'instant. Active le replay et appuie sur F" + (appController.config.saveKey - 111) + ", ou importe une vidéo."
        color: Theme.secondary
        wrapMode: Text.Wrap
        Layout.fillWidth: true
    }
    GridView {
        id: gallery
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        model: clipLibrary
        cellWidth: page.listMode ? width : width / Math.max(1, Math.floor(width / 290))
        cellHeight: page.listMode ? 78 : cellWidth * 0.5625 + 70
        ScrollBar.vertical: ScrollBar {}
        reuseItems: true
        cacheBuffer: 600
        delegate: Item {
            required property int index
            required property int clipId
            required property string clipName
            required property string filePath
            required property url thumbnail
            required property real duration
            required property var fileSize
            required property string created
            required property bool favorite
            required property bool missing
            required property string game
            required property string tags
            width: gallery.cellWidth - 14
            height: gallery.cellHeight - 14
            Rectangle {
                anchors.fill: parent
                color: hover.hovered ? Theme.raised : Theme.surface
                radius: 10
            }
            Rectangle {
                x: image.x
                y: image.y
                width: image.width
                height: image.height
                radius: page.listMode ? 6 : 0
                color: "#07080d"
                visible: image.status !== Image.Ready
            }
            Image {
                id: image
                x: page.listMode ? 8 : 0
                y: page.listMode ? (parent.height - height) / 2 : 0
                width: page.listMode ? 88 : parent.width
                height: width * 0.5625
                sourceSize.width: 480
                sourceSize.height: 270
                cache: true
                source: thumbnail
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }
            Text {
                visible: !page.listMode && image.status !== Image.Ready
                anchors.centerIn: image
                text: missing ? "Fichier manquant" : "Miniature en préparation"
                color: Theme.secondary
                font.pixelSize: 12
            }
            Rectangle {
                visible: !page.listMode
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8
                width: time.implicitWidth + 12
                height: 24
                radius: 4
                color: "#d0101119"
                Text {
                    id: time
                    anchors.centerIn: parent
                    text: Theme.duration(duration)
                    color: Theme.text
                    font.family: Theme.mono
                    font.pixelSize: 12
                }
            }
            Column {
                anchors.left: page.listMode ? image.right : parent.left
                anchors.right: menuButton.left
                anchors.bottom: page.listMode ? undefined : parent.bottom
                anchors.verticalCenter: page.listMode ? parent.verticalCenter : undefined
                anchors.margins: 12
                spacing: 5
                Text {
                    width: parent.width
                    text: (favorite ? "★  " : "") + clipName
                    color: missing ? Theme.danger : Theme.text
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: missing ? "Fichier déplacé ou supprimé" : [game || "Vidéo locale", Theme.date(created), Theme.size(fileSize)].concat(page.listMode ? [Theme.duration(duration)] : []).filter(function (t) {
                        return t.length > 0;
                    }).join("  ·  ")
                    color: Theme.secondary
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }
            HoverHandler {
                id: hover
            }
            TapHandler {
                onTapped: if (!missing)
                    appController.selectClip(index)
            }
            Button {
                id: menuButton
                anchors.right: parent.right
                anchors.bottom: page.listMode ? undefined : parent.bottom
                anchors.verticalCenter: page.listMode ? parent.verticalCenter : undefined
                anchors.margins: 8
                width: 34
                height: 34
                hoverEnabled: true
                opacity: hover.hovered || hovered || page.listMode || context.visible ? 1 : 0
                Behavior on opacity {
                    NumberAnimation { duration: 120 }
                }
                Accessible.name: "Actions du clip"
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                background: Rectangle {
                    radius: 17
                    color: menuButton.down ? Theme.accentSurface : menuButton.hovered ? Theme.line : "#c0101119"
                    border.color: menuButton.activeFocus ? Theme.accent : "transparent"
                    border.width: 2
                }
                padding: 0
                contentItem: Item {
                    Row {
                        anchors.centerIn: parent
                        spacing: 3
                        Repeater {
                            model: 3
                            Rectangle {
                                width: 4
                                height: 4
                                radius: 2
                                color: Theme.text
                            }
                        }
                    }
                }
                onClicked: openMenu(clipId, clipName, filePath, favorite, tags, menuButton, 0, height + 4)
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: function (point) {
                    openMenu(clipId, clipName, filePath, favorite, tags, parent, point.position.x, point.position.y);
                }
            }
        }
    }
    function openMenu(id, name, path, favorite, tags, anchorItem, x, y) {
        page.contextClip = {
            id: id,
            name: name,
            path: path,
            favorite: favorite,
            tags: tags
        };
        context.popup(anchorItem, x, y);
    }
    CMenu {
        id: context
        CMenuItem {
            text: page.contextClip.favorite ? "Retirer des favoris" : "Ajouter aux favoris"
            onTriggered: clipLibrary.favorite(page.contextClip.id, !page.contextClip.favorite)
        }
        CMenuItem {
            text: "Renommer"
            onTriggered: {
                editName.text = page.contextClip.name;
                rename.open();
                editName.forceActiveFocus();
                editName.selectAll();
            }
        }
        CMenuItem {
            text: "Modifier les tags"
            onTriggered: {
                editTags.text = page.contextClip.tags;
                tagDialog.open();
                editTags.forceActiveFocus();
            }
        }
        CMenuItem {
            text: "Copier le chemin"
            onTriggered: appController.copyPath(page.contextClip.path)
        }
        CMenuItem {
            text: "Afficher dans l'Explorateur"
            onTriggered: appController.reveal(page.contextClip.path)
        }
        CMenuSeparator {}
        CMenuItem {
            text: "Mettre à la corbeille…"
            danger: true
            onTriggered: deletion.open()
        }
    }
    CDialog {
        id: deletion
        title: "Mettre ce clip à la corbeille ?"
        body: "« " + (page.contextClip.name || "") + " » sera déplacé dans la corbeille Windows. Tu pourras le restaurer depuis l'Explorateur."
        acceptText: "Mettre à la corbeille"
        danger: true
        onAccepted: clipLibrary.remove(page.contextClip.id, true)
    }
    CDialog {
        id: rename
        title: "Renommer le clip"
        acceptText: "Enregistrer"
        acceptEnabled: editName.text.trim().length > 0
        onAccepted: clipLibrary.rename(page.contextClip.id, editName.text.trim())
        CField {
            id: editName
            Layout.fillWidth: true
            onAccepted: rename.submit()
        }
    }
    CDialog {
        id: tagDialog
        title: "Tags du clip"
        body: "Sépare les tags par des virgules."
        acceptText: "Enregistrer"
        onAccepted: clipLibrary.tags(page.contextClip.id, editTags.text)
        CField {
            id: editTags
            Layout.fillWidth: true
            placeholderText: "victoire, équipe, finale"
            onAccepted: tagDialog.submit()
        }
    }
}
