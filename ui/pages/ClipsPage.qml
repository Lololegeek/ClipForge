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
    // Multi-selection: clip id -> true. Reassigned on every change so bindings update.
    property bool selecting: false
    property var picked: ({})
    readonly property var pickedIds: Object.keys(picked).map(Number)
    function togglePick(id) {
        var p = Object.assign({}, picked);
        if (p[id])
            delete p[id];
        else
            p[id] = true;
        picked = p;
    }
    function pickAll() {
        var p = {};
        for (var i = 0; i < gallery.count; ++i)
            p[clipLibrary.get(i).id] = true;
        picked = p;
    }
    function endSelection() {
        selecting = false;
        picked = {};
    }
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
        CCombo {
            id: gameFilter
            implicitWidth: 200
            textRole: "text"
            model: [{
                    text: "Tous les jeux",
                    value: "*"
                }].concat(clipLibrary.gameList.map(function (g) {
                return {
                    text: (g.game || "Sans jeu") + "  (" + g.count + ")",
                    value: g.game
                };
            }))
            onActivated: clipLibrary.filterGame(model[currentIndex].value)
        }
        CheckBox {
            id: onlyFav
            text: "Favoris"
            onToggled: page.applyFilter()
        }
        ActionButton {
            text: page.selecting ? "Terminer" : "Sélectionner"
            primary: page.selecting
            onClicked: page.selecting ? page.endSelection() : page.selecting = true
        }
        ActionButton {
            text: page.listMode ? "Grille" : "Liste"
            onClicked: page.listMode = !page.listMode
        }
    }
    Rectangle {
        visible: page.selecting
        Layout.fillWidth: true
        implicitHeight: 52
        radius: 8
        color: Theme.accentSurface
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 8
            spacing: 8
            Text {
                text: page.pickedIds.length === 0 ? "Clique sur des clips pour les sélectionner" : page.pickedIds.length + (page.pickedIds.length > 1 ? " clips sélectionnés" : " clip sélectionné")
                color: Theme.text
                Layout.fillWidth: true
            }
            ActionButton {
                text: "Tout sélectionner"
                onClicked: page.pickAll()
            }
            ActionButton {
                text: "Favoris"
                enabled: page.pickedIds.length > 0
                onClicked: clipLibrary.favoriteMany(page.pickedIds, true)
            }
            ActionButton {
                text: "Retirer des favoris"
                enabled: page.pickedIds.length > 0
                onClicked: clipLibrary.favoriteMany(page.pickedIds, false)
            }
            ActionButton {
                text: "Ajouter un tag"
                enabled: page.pickedIds.length > 0
                onClicked: {
                    batchTagField.text = "";
                    batchTag.open();
                    batchTagField.forceActiveFocus();
                }
            }
            ActionButton {
                text: "Corbeille…"
                danger: true
                enabled: page.pickedIds.length > 0
                onClicked: batchDelete.open()
            }
        }
    }
    Text {
        visible: clipLibrary.count === 0
        text: search.text.length > 0 || onlyFav.checked || gameFilter.currentIndex > 0 ? "Aucun clip ne correspond à ces filtres." : "Aucun clip pour l'instant. Active le replay et appuie sur F" + (appController.config.saveKey - 111) + ", ou importe une vidéo."
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
            readonly property bool picked: !!page.picked[clipId]
            Rectangle {
                anchors.fill: parent
                color: hover.hovered ? Theme.raised : Theme.surface
                radius: 10
                border.color: Theme.accent
                border.width: parent.picked ? 2 : 0
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
                onTapped: {
                    if (page.selecting)
                        page.togglePick(clipId);
                    else if (!missing)
                        appController.selectClip(index);
                }
            }
            Rectangle {
                visible: page.selecting
                x: image.x + 8
                y: image.y + 8
                width: 22
                height: 22
                radius: 5
                color: parent.picked ? Theme.accent : "#c0101119"
                border.color: parent.picked ? Theme.accent : Theme.text
                border.width: 2
                Text {
                    anchors.centerIn: parent
                    visible: parent.parent.picked
                    text: "✓"
                    color: "#101119"
                    font.pixelSize: 14
                    font.bold: true
                }
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
        id: batchDelete
        title: "Mettre " + page.pickedIds.length + " clip(s) à la corbeille ?"
        body: "Les fichiers seront déplacés dans la corbeille Windows. Tu pourras les restaurer depuis l'Explorateur."
        acceptText: "Mettre à la corbeille"
        danger: true
        onAccepted: {
            clipLibrary.removeMany(page.pickedIds);
            page.picked = {};
        }
    }
    CDialog {
        id: batchTag
        title: "Ajouter un tag à " + page.pickedIds.length + " clip(s)"
        body: "Le tag s'ajoute à ceux déjà présents."
        acceptText: "Ajouter"
        acceptEnabled: batchTagField.text.trim().length > 0
        onAccepted: clipLibrary.tagMany(page.pickedIds, batchTagField.text.trim())
        CField {
            id: batchTagField
            Layout.fillWidth: true
            placeholderText: "finale"
            onAccepted: batchTag.submit()
        }
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
