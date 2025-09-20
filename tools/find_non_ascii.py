import os
import sys

def find_non_ascii_in_files():
    """
    Scans all .h and .cpp files in the ../src directory for non-ASCII characters.

    For each file containing non-ASCII characters, it prints the file path,
    line number, the content of the line, and a hexadecimal representation of the line.
    """
    # The script is expected to be in the 'tools' directory, so we look for 'src' one level up.
    try:
        script_dir = os.path.dirname(os.path.realpath(__file__))
        src_dir = os.path.abspath(os.path.join(script_dir, '..', 'src'))
    except NameError:
        # Handle case where __file__ is not defined (e.g., interactive interpreter)
        script_dir = os.getcwd()
        src_dir = os.path.abspath(os.path.join(script_dir, '..', 'src'))


    if not os.path.isdir(src_dir):
        print(f"Error: Source directory not found at '{src_dir}'", file=sys.stderr)
        sys.exit(1)

    print(f"Scanning for non-ASCII characters in: {src_dir}")

    for filename in sorted(os.listdir(src_dir)):
        if not (filename.endswith('.h') or filename.endswith('.cpp')):
            continue

        filepath = os.path.join(src_dir, filename)
        if not os.path.isfile(filepath):
            continue

        try:
            with open(filepath, 'rb') as f:
                content_bytes = f.read()
        except IOError as e:
            print(f"Error reading file {filepath}: {e}", file=sys.stderr)
            continue

        relative_path = os.path.relpath(filepath, os.path.join(src_dir, '..'))

        # Replace some special characters with standard ones.
        modified_content = content_bytes.replace(b'\xe2\x80\x91', b'-') # Unicode Character 'NON-BREAKING HYPHEN' (U+2011)
        modified_content = modified_content.replace(b'\xe2\x80\x93', b'-') # Unicode Character 'EN DASH' (U+2013)
        modified_content = modified_content.replace(b'\xe2\x80\x94', b'-') # Unicode Character 'EM DASH' (U+2014)
        modified_content = modified_content.replace(b'\xc2\xa0', b' ') # Unicode no-break space (0xC2 0xA0)
        if content_bytes != modified_content:
            print(f"Replacing U+2013 (EN DASH) with '-' in: {relative_path}")
            try:
                with open(filepath, 'wb') as f:
                    f.write(modified_content)
                # Use the modified content for the rest of the script
                content_bytes = modified_content
            except IOError as e:
                print(f"Error writing changes to file {filepath}: {e}", file=sys.stderr)
                continue

        content_for_processing = content_bytes
        if content_bytes.startswith(b'\xef\xbb\xbf'):
            content_for_processing = content_bytes[3:]
        else:
            # Do not warn for empty files, they don't need a BOM.
            if content_bytes:
                print(f"Warning: File {relative_path} is missing UTF-8 BOM.")

        lines = content_for_processing.splitlines()
            
        file_header_printed = False
        for i, line_bytes in enumerate(lines):
            # Check for any byte value > 127 (outside of 7-bit ASCII range)
            if any(b > 127 for b in line_bytes):
                if not file_header_printed:
                    # Use relative path for cleaner output
                    print(f"\n--- File: {relative_path} ---")
                    file_header_printed = True

                line_num = i + 1
                
                # Attempt to decode the line for a readable representation.
                # Try UTF-8 first, then fallback to cp1250 for legacy Czech encoding.
                line_text = ""
                try:
                    line_text = line_bytes.decode('utf-8').rstrip('\r\n')
                except UnicodeDecodeError:
                    try:
                        line_text = line_bytes.decode('cp1250').rstrip('\r\n')
                    except UnicodeDecodeError:
                        # If both decodings fail, mark it as undecodable.
                        line_text = "[Could not decode line content]"
                
                print(f"  Line {line_num}: {line_text}")
                # Find and print only the non-ASCII sequences in hex.
                non_ascii_sequences = []
                current_sequence = bytearray()
                for byte in line_bytes:
                    if byte > 127:
                        current_sequence.append(byte)
                    else:
                        if current_sequence:
                            non_ascii_sequences.append(current_sequence.hex())
                            current_sequence = bytearray()
                if current_sequence:
                    non_ascii_sequences.append(current_sequence.hex())

                if non_ascii_sequences:
                    print(f"    Hex: {', '.join(non_ascii_sequences)}")

if __name__ == '__main__':
    find_non_ascii_in_files() 