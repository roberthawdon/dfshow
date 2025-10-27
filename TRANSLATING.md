# Help Translate DF-SHOW

Thank you for your interest in helping translate DF-SHOW! Your contributions will help make this powerful file management tool accessible to users around the world.

## About DF-SHOW

DF-SHOW (Directory File Show) is a Unix-like rewrite of applications from Larry Kroeker's DF-EDIT, providing users with an interactive file browser and management tool. It allows users to view, copy, move, edit files and directories with features like file hunting, permission management, and much more.

## Why Translations Matter

By translating DF-SHOW, you're helping:
- Make the tool accessible to non-English speaking users
- Expand the reach of this useful file management utility
- Build a more inclusive open source community
- Preserve and share knowledge across language barriers

## How to Contribute

### Getting Started

1. **Check existing translations**: Look in the `po/` directory to see what languages are already available or in progress
2. **Choose your language**: Pick a language you're fluent in that isn't already completed
3. **Join the community**: Consider opening an issue to let us know you're working on a translation to avoid duplication

### Translation Process

1. **Fork the repository** on GitHub
2. **Create a new branch** for your translation (e.g., `translation-spanish` or `locale-es`)
3. **Set up your environment**:
   ```bash
   # Install gettext tools if not already available
   sudo apt-get install gettext  # On Debian/Ubuntu
   brew install gettext          # On macOS
   ```
4. **Work with PO files**:
   - **New language**: Copy the template file `po/dfshow.pot` to `po/[language_code].po`
   - **Existing language**: Edit the existing `po/[language_code].po` file
5. **Translate the content** using a PO editor or text editor
6. **Test your translation** (see testing section below)
7. **Submit a pull request** with your changes

### What Needs Translation

- **User interface messages**: Error messages, prompts, menu items
- **Help text**: Command descriptions and usage information
- **Status messages**: File operation confirmations and progress indicators
- **Error handling**: Clear error messages for various scenarios

## Translation Guidelines

### Working with PO Files

PO (Portable Object) files contain translatable strings in a structured format ⁴⁸. Each entry looks like:

```po
#: src/show.c:123
msgid "File not found"
msgstr ""
```

- `msgid`: The original English text (don't change this)
- `msgstr`: Your translation goes here
- Comments starting with `#:` show where the string is used in the code

### General Principles

- **Accuracy**: Ensure translations convey the correct meaning
- **Consistency**: Use the same terms for the same concepts throughout
- **Cultural appropriateness**: Adapt content to local conventions when needed
- **Technical precision**: Keep technical terms accurate and understandable

### Style Guidelines

- Use formal or informal tone consistently (match the original English tone)
- Preserve placeholders like `%s`, `%d`, `%c`, etc. - these will be replaced with actual values
- Don't translate:
  - Command names (like `show`, `sf`)
  - File paths and extensions
  - Configuration keys
  - Format specifiers (`%s`, `%d`, etc.)

### Recommended Tools

- **Poedit**: User-friendly graphical PO editor (https://poedit.net/)
- **Lokalize**: KDE's translation tool
- **Gtranslator**: GNOME translation editor
- **Text editor**: Any editor can work with PO files directly

## File Structure

```
po/
├── dfshow.pot      # Template file (don't edit directly)
├── en.po           # English (if exists)
├── es.po           # Spanish
├── fr.po           # French
├── de.po           # German
├── pt.po           # Portuguese
└── your-lang.po    # Your language
```

### Language Codes

Use standard ISO 639-1 language codes:
- `es` - Spanish
- `fr` - French  
- `de` - German
- `pt` - Portuguese
- `it` - Italian
- `ja` - Japanese
- `zh` - Chinese
- `ru` - Russian

For regional variants, use the format `language_COUNTRY`:
- `pt_BR` - Brazilian Portuguese
- `zh_CN` - Simplified Chinese
- `zh_TW` - Traditional Chinese

## Creating a New Translation

1. **Copy the template**:
   ```bash
   cp po/dfshow.pot po/[your_language_code].po
   ```

2. **Edit the header** in your new PO file:
   ```po
   "Language: [your_language_code]\n"
   "Last-Translator: Your Name <your.email@example.com>\n"
   "Language-Team: [Language] <team@example.com>\n"
   ```

3. **Translate the strings**: Fill in the `msgstr` fields with your translations

4. **Update the Makefile** (if needed): Add your language code to the build system

## Testing Your Translation

### Compile and Test
```bash
# Compile your PO file to test for syntax errors
msgfmt -c po/your_language.po

# If the project uses autotools/make:
make
LANG=your_language_code.UTF-8 ./show

# Test specific functionality to ensure translations appear correctly
```

### Validation Checklist
- [ ] All `msgstr` entries are filled (no empty translations)
- [ ] Placeholders (`%s`, `%d`, etc.) are preserved
- [ ] Text fits reasonably in the interface
- [ ] Special characters display correctly
- [ ] Plural forms work correctly (if applicable)

## Updating Existing Translations

When the source code changes, the POT template is updated. To update your translation:

```bash
# Update your PO file with new strings from the template
msgmerge -U po/your_language.po po/dfshow.pot

# Look for "fuzzy" entries that need review
# Translate any new untranslated strings
```

## Languages We'd Love to See

While we welcome translations into any language, these would be particularly valuable:
- Spanish (Español) - `es`
- French (Français) - `fr`
- German (Deutsch) - `de`
- Portuguese (Português) - `pt`
- Italian (Italiano) - `it`
- Japanese (日本語) - `ja`
- Chinese (中文) - `zh`
- Russian (Русский) - `ru`

## Getting Help

- **Questions?** Open an issue with the `translation` label
- **Need context?** Don't hesitate to ask about unclear strings - the `#:` comments show where each string is used
- **Want to coordinate?** Reach out to discuss larger translation efforts
- **PO file issues?** Check the [GNU gettext manual](https://www.gnu.org/software/gettext/manual/) ⁵⁶

## Recognition

All translation contributors will be:
- Listed in the project's CONTRIBUTORS file
- Credited in release notes
- Mentioned in the README
- Listed in the PO file headers as translators

## Technical Requirements

- Basic familiarity with Git and GitHub
- Text editor or PO editor (Poedit recommended for beginners)
- gettext tools for testing (optional but helpful)
- Native or near-native fluency in your target language

## Maintenance

Translations need ongoing maintenance as the project evolves ⁵⁴. If you contribute a translation:
- Consider subscribing to repository notifications
- We'll tag you in issues related to your language
- Help us keep translations up to date when new strings are added
- Review "fuzzy" entries when the POT template is updated

## Code of Conduct

Please note that this project follows a code of conduct. By participating in translations, you agree to abide by its terms and help maintain a welcoming environment for all contributors.

---

**Ready to start?** Fork the repository, copy `po/dfshow.pot` to `po/[your_language].po`, and begin translating! If you have any questions, don't hesitate to open an issue or reach out to the maintainers.

Thank you for helping make DF-SHOW accessible to users worldwide!
