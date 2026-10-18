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
            placeholderText: "Rechercher un nom, un jeu ou un tag"
            onTextChanged: clipLibrary.filter(text, sort.model[sort.currentIndex].value, onlyFav.checked)
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
            onActivated: clipLibrary.filter(search.text, model[currentIndex].value, onlyFav.checked)
        }
        CheckBox {
            id: onlyFav
            text: "Favoris"
            onToggled: clipLibrary.filter(search.text, sort.model[sort.currentIndex].value, checked)
        }
        ActionButton {
            text: page.listMode ? "Grille" : "Liste"
            onClicked: page.listMode = !page.listMode
        }
    }
    Text {
        visible: clipLibrary.count === 0
        text: "Aucun clip trouvé. Sauvegarde un replay ou importe une vidéo."
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
        delegate: Item {
            required property int index
            required property int clipId
            required property string clipName
            required property string filePath
            required property url thumbnail
            required property real duration
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
            Image {
                id: image
                visible: !page.listMode
                anchors.top: parent.top
                width: parent.width
                height: width * 0.5625
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
                    text: Math.floor(duration / 60) + ":" + String(Math.floor(duration % 60)).padStart(2, "0")
                    color: Theme.text
                    font.family: Theme.mono
                    font.pixelSize: 12
                }
            }
            Column {
                anchors.left: parent.left
                anchors.right: menuButton.left
                anchors.bottom: parent.bottom
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
                    text: missing ? "Fichier déplacé ou supprimé" : (game || "Vidéo locale") + (page.listMode ? " · " + Math.round(duration) + " s" : "")
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
            ActionButton {
                id: menuButton
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 8
                implicitWidth: 34
                text: "⋯"
                onClicked: {
                    page.contextClip = {
                        id: clipId,
                        name: clipName,
                        path: filePath,
                        favorite: favorite,
                        tags: tags
                    };
                    context.open();
                }
            }
        }
    }
    Menu {
        id: context
        MenuItem {
            text: page.contextClip.favorite ? "Retirer des favoris" : "Ajouter aux favoris"
            onTriggered: clipLibrary.favorite(page.contextClip.id, !page.contextClip.favorite)
        }
        MenuItem {
            text: "Renommer"
            onTriggered: {
                editName.text = page.contextClip.name;
                rename.open();
            }
        }
        MenuItem {
            text: "Modifier les tags"
            onTriggered: {
                editTags.text = page.contextClip.tags;
                tagDialog.open();
            }
        }
        MenuItem {
            text: "Copier le chemin"
            onTriggered: appController.copyPath(page.contextClip.path)
        }
        MenuItem {
            text: "Afficher dans l'Explorateur"
            onTriggered: appController.reveal(page.contextClip.path)
        }
        MenuSeparator {}
        MenuItem {
            text: "Mettre à la corbeille…"
            onTriggered: deletion.open()
        }
    }
    Dialog {
        id: deletion
        title: "Mettre ce clip à la corbeille ?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.No
        Label {
            text: page.contextClip.name || ""
        }
        onAccepted: clipLibrary.remove(page.contextClip.id, true)
    }
    Dialog {
        id: rename
        title: "Renommer le clip"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Save | Dialog.Cancel
        CField {
            id: editName
            width: 320
        }
        onAccepted: clipLibrary.rename(page.contextClip.id, editName.text)
    }
    Dialog {
        id: tagDialog
        title: "Tags du clip"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Save | Dialog.Cancel
        CField {
            id: editTags
            width: 320
            placeholderText: "victoire, équipe, finale"
        }
        onAccepted: clipLibrary.tags(page.contextClip.id, editTags.text)
    }
}
