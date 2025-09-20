import os
import sys
import re
import csv

try:
    import plotly.graph_objects as go
    from wordfreq import top_n_list
    from unidecode import unidecode
except ImportError:
    print("The 'wordfreq', 'unidecode', and 'plotly' libraries are required. Please install them by running: pip install wordfreq unidecode plotly", file=sys.stderr)
    sys.exit(1)

import tree_sitter_cpp as ts_cpp
import tree_sitter_c as ts_c
from tree_sitter import Language, Parser

# New imports for language detection via word frequency
try:
    from wordfreq import top_n_list
    from unidecode import unidecode
except ImportError:
    print("The 'wordfreq' and 'unidecode' libraries are required. Please install them by running: pip install wordfreq unidecode", file=sys.stderr)
    sys.exit(1)

# Build word-frequency based vocabularies for language classification
WORD_RE = re.compile(r"[A-Za-z]+")

def _build_word_set(lang: str, count: int) -> set[str]:
    """Return a set of the *count* most-common words for *lang* without diacritics."""
    words: set[str] = set()
    for w in top_n_list(lang, count):
        w_ascii = re.sub('[^A-Za-z]', '', unidecode(w)).lower()
        if w_ascii:
            words.add(w_ascii)
    return words

# Tune the sizes – large list for Czech, slightly smaller for English to save memory
CS_WORDS = _build_word_set('cs', 500000)
EN_WORDS = _build_word_set('en', 200000)
# remove overlap so words present in both are considered English (safer default)
CS_WORDS -= EN_WORDS

# Define languages and parsers for C and C++
CPP_LANGUAGE = Language(ts_cpp.language())
#C_LANGUAGE = Language(ts_c.language())

#c_parser = Parser(C_LANGUAGE)
cpp_parser = Parser(CPP_LANGUAGE)

def get_parser(file_path):
    """Returns the appropriate tree-sitter parser based on file extension."""
    if file_path.endswith('.c'):
        #return c_parser
        return cpp_parser
    # Use C++ parser for .h, .rh, .cpp, .rc as it generally handles C-style comments
    return cpp_parser

def extract_comments_from_file(file_path):
    """Extracts all comments from a given source file."""
    parser = get_parser(file_path)
    try:
        # Use error handling for different file encodings
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as file:
            code = file.read()
    except Exception as e:
        print(f"Error reading file {file_path}: {e}", file=sys.stderr)
        return []

    tree = parser.parse(bytes(code, 'utf8'))
    comments = []
    
    def find_comments_recursive(node):
        """Recursively traverses the syntax tree to find comment nodes."""
        if 'comment' in node.type:
            comments.append(code[node.start_byte:node.end_byte])
        for child in node.children:
            find_comments_recursive(child)
            
    find_comments_recursive(tree.root_node)
    return comments

def _token_counts(text: str) -> tuple[int, int]:
    """Return counts of (czech_like, english_like) tokens in *text*."""
    cs_cnt = en_cnt = 0
    for token in WORD_RE.findall(text):
        t = unidecode(token).lower()
        if t in CS_WORDS:
            cs_cnt += 1
        elif t in EN_WORDS:
            en_cnt += 1
    return cs_cnt, en_cnt

def classify_language(comment: str) -> str:
    """Classify comment text as 'cs', 'en', or 'unknown' using token overlap.

    Heuristic:
      • At least one recognised token must be present.
      • If the ratio of Czech-like to English-like tokens ≥ 2, label 'cs'.
      • If the ratio of English-like to Czech-like tokens ≥ 2, label 'en'.
      • Otherwise, 'unknown'.
    """
    cs_cnt, en_cnt = _token_counts(comment)

    total = cs_cnt + en_cnt
    if total == 0:
        return 'unknown'

    # decide by dominance, with small threshold to avoid ties
    if cs_cnt >= en_cnt * 2:
        return 'cs'
    if en_cnt >= cs_cnt * 2:
        return 'en'
    if cs_cnt >= en_cnt:
        return 'cs'
    if en_cnt >= cs_cnt:
        return 'en'
    return 'unknown'

def main():
    """
    Main function to drive the comment extraction and analysis.
    """
    # The script is expected to be in a subdirectory (e.g., 'tools'), 
    # so the project root is the parent directory.
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    #project_root = r'D:\Work\Downloads\salamander\salamander\src'
    
    output_filename = '../comments.txt'
    output_file_path = os.path.join(project_root, output_filename)
    target_extensions = ('.h', '.rh', '.c', '.cpp', '.rc')
    
    # Directories to exclude from analysis
    excluded_dirs = {
        'src/common/dep',
        'src/sfx7zip/7zip',
        'src/sfx7zip/branch',
        'src/sfx7zip/lzma',
        'src/plugins/7zip/7za/c',
        'src/plugins/7zip/7za/cpp',
        'src/plugins/automation/generated',
        'src/plugins/checksum/tomcrypt',
        'src/plugins/ftp/openssl',
        'src/plugins/ieviewer/cmark-gfm',
        'src/plugins/mmviewer/ogg/vorbis',
        'src/plugins/mmviewer/wma/wmsdk',
        'src/plugins/pictview/exif/libexif',
        'src/plugins/pictview/exif/libjpeg',
        'src/plugins/pictview/twain',
        'src/plugins/portables/wtl',
        'src/plugins/shared/sqlite',
        'src/plugins/unchm/chmlib',
        'src/plugins/winscp/core',
        'src/plugins/winscp/forms',
        'src/plugins/winscp/packages',
        'src/plugins/winscp/putty',
        'src/plugins/winscp/resource',
        'src/plugins/winscp/windows/',
        'tree-sitter-grammars',
        # TEMP
        'src/common',
        'src/plugins',
        'src/reglib',
        'src/salmon',
        'src/salopen',
        'src/salspawn',
        'src/shellext',
        'src/setup',
        'src/sfx7zip',
        'src/translator',
        'src/tserver',
        'tools',
    }
    
    # Dictionary to store statistics for each file
    file_stats = {}

    print(f"Starting comment extraction from '{project_root}'...")
    with open(output_file_path, 'w', encoding='utf-8') as out_file:
        for root, dirs, files in os.walk(project_root):
            # Exclude common version control directories
            dirs[:] = [d for d in dirs if d not in ['.git', '.svn']]

            dir_path_relative = os.path.relpath(root, project_root)
            # Normalize path for consistent comparison
            normalized_path = dir_path_relative.replace(os.path.sep, '/')

            # Check if the current directory should be excluded
            if any(normalized_path.startswith(ex_dir) for ex_dir in excluded_dirs):
                dirs[:] = []  # Don't traverse into subdirectories
                continue      # Skip processing files in this directory

            for filename in files:
                if filename.endswith(target_extensions):
                    file_path = os.path.join(root, filename)
                    relative_path = os.path.relpath(file_path, project_root).replace(os.path.sep, '/')
                    
                    out_file.write(f"--- File: {relative_path} ---\n")
                    
                    comments = extract_comments_from_file(file_path)
                    
                    file_cs_size = 0
                    file_en_size = 0

                    for comment in comments:
                        comment_text = comment.strip()
                        if not comment_text:
                            continue
                        
                        out_file.write(comment_text + '\n')
                        
                        lang = classify_language(comment_text)
                        comment_size = len(comment_text.encode('utf-8'))
                        
                        if lang == 'cs':
                            file_cs_size += comment_size
                        elif lang == 'en':
                            file_en_size += comment_size
                    
                    if file_cs_size > 0 or file_en_size > 0:
                        file_stats[relative_path] = {'cs': file_cs_size, 'en': file_en_size}
                    
                    out_file.write('\n')

    print(f"Comment extraction complete. Results saved to '{output_file_path}'")

    # The text-based report is now generated inside generate_treemap for simplicity
    # with the aggregated data used by the treemap itself.

    # --- Generate Treemap ---
    generate_treemap(file_stats, project_root)

def generate_treemap(file_stats, project_root):
    """Generates and saves a treemap visualization and a CSV report of comment statistics."""

    # --- Generate CSV Report ---
    csv_filename = '../comments_stats.csv'
    csv_filepath = os.path.join(project_root, csv_filename)
    try:
        with open(csv_filepath, 'w', newline='', encoding='utf-8') as csvfile:
            csv_writer = csv.writer(csvfile)
            # Write header
            csv_writer.writerow(['File Path', 'English Comments (KB)', 'Czech Comments (KB)', 'English Translation (%)'])

            # Write data rows for each file
            for path, stats in sorted(file_stats.items()):
                cs_size = stats['cs']
                en_size = stats['en']
                total_size = cs_size + en_size

                cs_kb = cs_size / 1024
                en_kb = en_size / 1024
                
                en_percentage = (en_size / total_size * 100) if total_size > 0 else 0

                csv_writer.writerow([path, f'{en_kb:.2f}', f'{cs_kb:.2f}', f'{en_percentage:.1f}'])
        
        print(f"\nCSV report saved to '{csv_filepath}'")
    except Exception as e:
        print(f"\nError saving CSV report: {e}", file=sys.stderr)
    
    # aggregated_stats will contain files and directories
    aggregated_stats = file_stats.copy()

    # Create entries for all parent directories from file paths
    for path in file_stats.keys():
        parent_path = os.path.dirname(path).replace(os.path.sep, '/')
        while parent_path and parent_path != '.':
            if parent_path not in aggregated_stats:
                aggregated_stats[parent_path] = {'cs': 0, 'en': 0}
            parent_path = os.path.dirname(parent_path).replace(os.path.sep, '/')
    
    if '.' not in aggregated_stats:
         aggregated_stats['.'] = {'cs': 0, 'en': 0}

    # Aggregate stats up the tree from files to parent directories
    for path in sorted(aggregated_stats.keys(), key=lambda p: p.count('/'), reverse=True):
        if path == '.':
            continue
        stats = aggregated_stats[path]
        if os.path.dirname(path) == path: # root case
            continue
        parent_path = os.path.dirname(path).replace(os.path.sep, '/')
        if not parent_path:
            parent_path = '.'
        
        if parent_path in aggregated_stats:
            aggregated_stats[parent_path]['cs'] += stats['cs']
            aggregated_stats[parent_path]['en'] += stats['en']

    # The console output has been replaced by the CSV file generation.

    # Prepare data for Plotly
    ids = []
    labels = []
    parents = []
    values = []
    marker_colors = []
    customdata = []
    
    if '.' in aggregated_stats:
        aggregated_stats['Project Root'] = aggregated_stats.pop('.')

    for path, stats in sorted(aggregated_stats.items()):
        total_size = stats['cs'] + stats['en']
        
        if total_size == 0 and path != 'Project Root':
            continue

        parent_path = os.path.dirname(path).replace(os.path.sep, '/')
        if path == 'Project Root':
            label = 'Project Root'
            parent = ''
        else:
            label = os.path.basename(path)
            if parent_path in ['', '.']:
                parent = 'Project Root'
            else:
                parent = parent_path
        
        if parent and parent not in aggregated_stats:
            parent = 'Project Root'

        ids.append(path)
        labels.append(label)
        parents.append(parent)
        values.append(total_size)
        
        cs_ratio = stats['cs'] / total_size if total_size > 0 else 0.5
        
        # Color scale: Red (Czech) -> Gray (50/50) -> Green (English)
        if cs_ratio > 0.5:
            # More Czech: Interpolate from Gray to Red
            p = (cs_ratio - 0.5) * 2
            red = int(128 + p * 127)
            green = int(128 - p * 128)
            blue = int(128 - p * 128)
        else:
            # More English: Interpolate from Green to Gray
            p = cs_ratio * 2
            red = int(p * 128)
            green = int(160 - p * 52)  # Use a less saturated green
            blue = int(p * 128)
        marker_colors.append(f'rgb({red},{green},{blue})')
        
        cs_kb = stats['cs'] / 1024
        en_kb = stats['en'] / 1024
        en_percentage = (stats['en'] / total_size * 100) if total_size > 0 else 0
        customdata.append([f"{cs_kb:.1f}", f"{en_kb:.1f}", f"{en_percentage:.1f}"])

    if not ids:
        print("\nNo comments found to generate a treemap.")
        return

    fig = go.Figure(go.Treemap(
        ids=ids,
        labels=labels,
        parents=parents,
        values=values,
        branchvalues='total',
        marker_colors=marker_colors,
        customdata=customdata,
        #textinfo="label+value",
        textfont=dict(color='white'),
        texttemplate="<b>%{label}</b><br>EN/CZ:<br>%{customdata[1]} / %{customdata[0]} KB<br>%{customdata[2]}%",
        hovertemplate='<b>%{id}</b><br>English/Czech: %{customdata[1]} / %{customdata[0]} KB<br>Translated: %{customdata[2]}%<extra></extra>',
    ))

    fig.update_layout(
        #title_text='Comment Language Distribution (Green = EN, Red = CS)',
        margin = dict(t=3, l=3, r=3, b=3)
    )

    treemap_filename = '../comments_treemap.html'
    treemap_filepath = os.path.join(project_root, treemap_filename)
    try:
        fig.write_html(treemap_filepath)
        print(f"\nTreemap visualization saved to '{treemap_filepath}'")
    except Exception as e:
        print(f"\nError saving treemap: {e}", file=sys.stderr)

if __name__ == '__main__':
    main()