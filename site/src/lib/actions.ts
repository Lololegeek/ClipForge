import { toast } from "sonner";
import { useDemo } from "@/lib/store";

export function saveReplayWithToast() {
  const clip = useDemo.getState().saveReplay();
  if (!clip) {
    toast("Le replay est à l’arrêt", {
      description: "Active le replay pour remplir le tampon, comme dans l’application.",
    });
    return;
  }
  toast.success(`Clip sauvegardé : ${clip.name}.mp4`, {
    description: `${clip.seconds} s · démo dans le navigateur, rien n’est enregistré`,
  });
}

export function toggleRecordingWithToast() {
  const st = useDemo.getState();
  st.toggleRecording();
  toast(st.recording ? "Enregistrement terminé" : "Enregistrement en cours", {
    description: st.recording
      ? "Le fichier MKV reste lisible même après un plantage."
      : "Appuie à nouveau sur F9 pour arrêter.",
  });
}

export function screenshotWithToast() {
  useDemo.getState().screenshot();
  toast("Capture d’écran enregistrée", {
    description: "F10 capture l’écran principal, pas l’image du jeu.",
  });
}
