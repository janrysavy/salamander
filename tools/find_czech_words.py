import re
from pathlib import Path
from wordfreq import top_n_list
from unidecode import unidecode

# Build sets of words without diacritics
def build_word_set(lang: str, count: int) -> set[str]:
    words = set()
    for w in top_n_list(lang, count):
        w_ascii = re.sub('[^A-Za-z]', '', unidecode(w)).lower()
        if w_ascii:
            words.add(w_ascii)
    return words

CS_WORDS = build_word_set('cs', 500000)
EN_WORDS = build_word_set('en', 200000)
CS_WORDS -= EN_WORDS  # remove words common with English

WORD_RE = re.compile(r"[A-Za-z]+")


def extract_czech_words(text: str) -> set[str]:
    """Return Czech words without diacritics found in the text."""
    words = set()
    for token in WORD_RE.findall(text):
        token_lower = token.lower()
        if token_lower in CS_WORDS:
            words.add(token_lower)
    return words


def main() -> None:
    src_dir = Path('D:/Source/OpenSal/salamander_jan/src')
    all_words = set()
    for path in sorted(src_dir.iterdir()): #rglob('*')
        if path.suffix in {'.cpp', '.h'} and path.is_file():
            try:
                content = path.read_text(encoding='utf-8', errors='ignore')
            except Exception:
                continue
            words = extract_czech_words(content)
            if words:
                all_words.update(words)
                print(f"{path}:")
                for word in sorted(words):
                    print(f"  {word}")
                print()

    print("\n--- Všechna nalezená česká slova (abecedně) ---")
    for word in sorted(all_words):
        print(word)
    print()


if __name__ == '__main__':
    main()