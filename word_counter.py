import re
import collections
import os

def normalize_char(c):
    replacements = {
        'á': 'a', 'č': 'c', 'ď': 'd', 'é': 'e', 'ě': 'e', 'í': 'i',
        'ň': 'n', 'ó': 'o', 'ř': 'r', 'š': 's', 'ť': 't', 'ú': 'u',
        'ů': 'u', 'ý': 'y', 'ž': 'z',
        'Á': 'a', 'Č': 'c', 'Ď': 'd', 'É': 'e', 'Ě': 'e', 'Í': 'i',
        'Ň': 'n', 'Ó': 'o', 'Ř': 'r', 'Š': 's', 'Ť': 't', 'Ú': 'u',
        'Ů': 'u', 'Ý': 'y', 'Ž': 'z'
    }
    return replacements.get(c, c)

def normalize_text(text):
    return "".join(normalize_char(c) for c in text.lower())

czech_words = [
    "a", "aby", "ale", "aniz", "ano", "asi", "az", "bez", "bude", "budem", "budes", "by", "byl", "byla",
    "byli", "bylo", "byt", "ci", "clanku", "co", "com", "coz", "cz", "das", "do", "email", "ho", "i", "jak",
    "jake", "jako", "je", "jeho", "jej", "jeji", "jejich", "jemuz", "jen", "jenz", "jeste", "jez", "ji",
    "jiz", "jine", "jiri", "jsem", "jses", "jsi", "jsme", "jsou", "jste", "k", "kam", "kde", "kdo", "kdyz",
    "ke", "ktery", "ktera", "ktere", "kteri", "kterou", "ku", "ma", "mate", "me", "meho", "mezi", "mi",
    "mit", "mne", "mnou", "muj", "mu", "muze", "my", "na", "nad", "nam", "napiste", "nas", "nase", "nasi",
    "ne", "nebo", "nebot", "necht", "nej", "nejsme", "neni", "net", "nez", "ni", "nic", "nove", "novy",
    "o", "od", "ode", "on", "org", "pak", "po", "pod", "podle", "pokud", "potom", "pouze", "prave",
    "pred", "pres", "pri", "pro", "proc", "proto", "protoze", "prvni", "pta", "s", "se", "si", "smime",
    "snad", "spolecnosti", "sve", "svuj", "svych", "svym", "svymi", "ta", "tak", "take", "takze", "tam",
    "tamhle", "tato", "te", "tedy", "tehdy", "ten", "tento", "teto", "tim", "timto", "to", "tohoto", "tom",
    "tomto", "totiz", "tu", "tuto", "tvuj", "ty", "tyto", "u", "uz", "v", "vam", "vas", "ve", "vedle",
    "vsak", "vsechen", "vy", "vzdy", "z", "za", "zatimco", "ze", "zpet", "zprava", "zpravy"
]

file_list = [
    "bitmap.cpp", "bitmap.h", "bugreprt.cpp", "cache.cpp", "cache.h", "callstk.cpp", "callstk.h",
    "cfgdlg.h", "codetbl.cpp", "codetbl.h", "color.cpp", "color.h", "consts.h", "dialogs.cpp",
    "dialogs.h", "dialogs2.cpp", "dialogs3.cpp", "dialogs4.cpp", "dialogs5.cpp", "dialogs6.cpp",
    "dialogse.cpp", "dialogsp.cpp", "drivelst.cpp", "drivelst.h", "editwnd.cpp", "editwnd.h",
    "edtlbwnd.cpp", "edtlbwnd.h", "execute.cpp", "execute.h", "filesbox.h", "filesbx1.cpp",
    "filesbx2.cpp", "filesmap.cpp", "fileswn0.cpp", "fileswn1.cpp", "fileswn2.cpp", "fileswn3.cpp",
    "fileswn4.cpp", "fileswn5.cpp", "fileswn6.cpp", "fileswn7.cpp", "fileswn8.cpp", "fileswn9.cpp",
    "fileswna.cpp", "fileswnb.cpp", "fileswnd.h", "filter.cpp", "filter.h", "find.cpp", "find.h",
    "finddlg1.cpp", "finddlg2.cpp", "geticon.cpp", "geticon.h", "gui.cpp", "gui.h", "icncache.cpp",
    "icncache.h", "iconlist.cpp", "iconlist.h", "inflate.cpp", "inflate.h", "jumplist.cpp",
    "jumplist.h", "keyboard.cpp", "logo.cpp", "logo.h", "mainwnd.h", "mainwnd1.cpp", "mainwnd2.cpp",
    "mainwnd3.cpp", "mainwnd4.cpp", "mainwnd5.cpp", "manifest.xml", "mapi.cpp", "mapi.h", "masks.cpp",
    "masks.h", "md5.cpp", "md5.h", "menu.h", "menu1.cpp", "menu2.cpp", "menu3.cpp", "menu4.cpp",
    "menubar.cpp", "ms_init.cpp", "msgbox.cpp", "olespy.cpp", "olespy.h", "pack.h", "pack1.cpp",
    "pack2.cpp", "pack3.cpp", "packac.cpp", "packers.cpp", "plugins.h", "plugins1.cpp",
    "plugins2.cpp", "plugins3.cpp", "plugins4.cpp", "precomp.cpp", "precomp.h", "pwdmngr.cpp",
    "pwdmngr.h", "regwork.cpp", "regwork.h", "resource.rh2", "safefile.cpp", "salamand.h",
    "salamand.rc", "salamand.rc2", "salamand.rh", "salamdr1.cpp", "salamdr2.cpp", "salamdr3.cpp",
    "salamdr4.cpp", "salamdr5.cpp", "salamdr6.cpp", "salamdr7.cpp", "salbzip2.cpp", "salinflt.cpp",
    "salinflt.h", "salmoncl.cpp", "salmoncl.h", "salshlib.cpp", "salshlib.h", "salzlib.cpp",
    "shares.cpp", "shellib.cpp", "shellib.h", "shellsup.cpp", "shexreg.c", "shexreg.h",
    "shiconov.cpp", "shiconov.h", "snooper.cpp", "snooper.h", "sort.cpp", "sort.h", "stswnd.cpp",
    "stswnd.h", "svg.cpp", "svg.h", "tabwnd.cpp", "tabwnd.h", "tasklist.cpp", "tasklist.h",
    "texts.rh2", "thumbnl.cpp", "thumbnl.h", "toolbar.h", "toolbar1.cpp", "toolbar2.cpp",
    "toolbar3.cpp", "toolbar4.cpp", "toolbar5.cpp", "toolbar6.cpp", "toolbar7.cpp", "toolbar8.cpp",
    "tooltip.cpp", "tooltip.h", "usermenu.h", "versinfo.cpp", "versinfo.h", "versinfo.rh2",
    "viewer.cpp", "viewer.h", "viewer2.cpp", "viewer3.cpp", "worker.cpp", "worker.h", "zip.cpp", "zip.h"
]

word_counts = collections.Counter()

for file_name in file_list:
    file_path = os.path.join("D:\\Source\\OpenSal\\salamander_jan\\src", file_name)
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            normalized_content = normalize_text(content)
            words = re.findall(r'\b[a-z]+\b', normalized_content)
            for word in words:
                if word in czech_words:
                    word_counts[word] += 1
    except FileNotFoundError:
        pass # Ignore files that are not found

for word, count in sorted(word_counts.items()):
    print(f"{word}: {count}")
