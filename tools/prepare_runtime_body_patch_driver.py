from pathlib import Path

path = Path("tools/apply_runtime_body_profiles_v01.py")
text = path.read_text(encoding="utf-8")
old = '''def replace_once(path: Path, old: str, new: str, label: str) -> None:\n    text = path.read_text(encoding="utf-8")\n    count = text.count(old)\n    if count != 1:\n        raise RuntimeError(f"{label}: expected exactly one match in {path}, found {count}")\n    path.write_text(text.replace(old, new, 1), encoding="utf-8")\n    print(f"[HAKUI BODY v0.1] {label}")\n'''
new = '''def replace_once(path: Path, old: str, new: str, label: str) -> None:\n    text = path.read_text(encoding="utf-8")\n    count = text.count(old)\n    if label == "add F6 runtime body-profile switch" and count >= 1:\n        # HakuiApp observes gamepad add/remove events before the general input\n        # bridge. The runtime body switch belongs immediately before the final\n        # general observeEvent call, so patch the last occurrence deliberately.\n        index = text.rfind(old)\n        text = text[:index] + new + text[index + len(old):]\n        path.write_text(text, encoding="utf-8")\n        print(f"[HAKUI BODY v0.1] {label}")\n        return\n    if count != 1:\n        raise RuntimeError(f"{label}: expected exactly one match in {path}, found {count}")\n    path.write_text(text.replace(old, new, 1), encoding="utf-8")\n    print(f"[HAKUI BODY v0.1] {label}")\n'''
if old not in text:
    raise RuntimeError("runtime body patch driver could not find replace_once definition")
path.write_text(text.replace(old, new, 1), encoding="utf-8")
print("[HAKUI BODY v0.1] patch driver prepared")
