# Repository Instructions for Codex

This repository includes guidelines for translating code comments. These rules should be applied to every request.

## Core Translation Rules

- Translate all Czech comments to English.
- Preserve the original file formatting and indentation.
- Do not modify the code itself; only translate comments.
- If you encounter typos in the Czech text, correct them in the English translation.
- Preserve paths containing slashes or backslashes exactly as they appear.
- Maintain consistency of translated terms throughout the repository.
- Use the adjacent code for context to produce accurate translations.
- If the volume of comments is too big, just translate what you can.
- Preserve all quotation marks or apostrophes used to denote parts of strings.
- Keep dashes and other similar formatting characters.

## Workflow For Each Translation Task

- Read the entire target file first to understand the module’s purpose and how comments relate to surrounding code.
- Locate paired headers, implementations, or nearby helper files to see how the symbols referenced in the comment are used.
- Use `rg`, `git grep`, or similar tools to search the repository for identifiers, Czech terminology, or related comments before deciding on an English phrasing.
- When documentation or UI texts exist in `doc/`, `help/`, or `translations/`, review them to align terminology and phrasing.
- After translating, re-read the diff to confirm that only comments changed and that spacing, alignment, and markers (such as `//`, `/*`, `*/`) remain untouched.

## Digging Deeper For Context

- Investigate function call sites, struct definitions, and enum values to infer the exact meaning of Czech phrases, especially when they use idioms or domain-specific abbreviations.
- Check initialization defaults, error messages, and surrounding logic to decide whether a translation should be literal, descriptive, or imperative.
- Use `git blame` or `git log` for the affected lines when historical intent or terminology is unclear; prior authorship notes can clarify ambiguous wording.
- If terminology differs across files, prefer the most recent or most widely used English equivalent in this repository to maintain consistency.

## Consistency And Terminology

- Maintain consistent translations for recurring UI labels, dialog names, menu commands, and internal concepts (for example, “panel”, “directory”, “archive”, “queue”).
- Mirror capitalization, punctuation, and formatting conventions already established for similar English comments in the project.
- Expand acronyms only when the code or documentation nearby spells them out; otherwise keep the established abbreviation.
- Normalize technical wording so that it matches C++ terminology (e.g., “pointer”, “buffer”, “invalid handle”) unless the surrounding code indicates a different nuance.
- For corrected typos or grammar, keep the meaning faithful to the intent of the original comment rather than substituting a new idea.

## Handling Ambiguity

- When the Czech source leaves room for interpretation, prefer the translation that best fits the observed behavior of the code instead of a literal word-for-word rendering.
- If uncertainty remains after checking context, add a brief translator note in your response (not in the code) explaining the assumption you made.
- Flag any comment whose meaning depends on yet-untranslated comments or documentation so follow-up work can align the wider context.

## Quality Checks Before Finishing

- Verify that no code tokens, string contents, or pragma directives were altered while editing comments.
- Ensure the English text reads naturally to an experienced C++ developer and avoids awkward literal translations.
- Re-run any formatting or linting checks requested by the user to confirm the file remains clean; do not introduce new tooling output into the repository.
- Summarize the changes for the user, including any open questions or assumptions, so they can review or provide clarification quickly.
