# Fixup code namespaces to be customizable:
# As run for fixing namespaces in codebase for Github issue #773

import subprocess, os

mx_root = r"."
clf_exe = r"clang-format"
git_exe = r"git"

trivial_remove = [b'-namespace MaterialX', b'-{', b'-}', b'-} // namespace MaterialX', b'-} // MaterialX']
trivial_adds = [b'+MATERIALX_NAMESPACE_BEGIN', b'+', b'+MATERIALX_NAMESPACE_END']

def apply_edits(allLines, source_line, added, removed):
    validate = False
    num_open = 0
    num_close = 0
    # If the edit is non-trivial, then validate afterwards.
    if len(added) != len(removed):
        validate = True
    for a, d in zip(added,removed):
        if d in trivial_remove and a in trivial_adds:
            allLines[source_line - 1] = a[1:].decode("utf-8") + "\n"
            if a == b'+MATERIALX_NAMESPACE_BEGIN':
                num_open += 1
            if a == b'+MATERIALX_NAMESPACE_END':
                num_close += 1
        else:
            validate = True
        source_line += 1
    return validate and num_open, num_open, num_close

def fix_namespaces(cur_file):
    seen_open = False
    num_open = 0
    num_close = 0
    with open(cur_file, "rt") as in_f:
        with open(cur_file+b".fixed", "wt") as out_f:
            for cur_line in in_f:
                if cur_line == "namespace MaterialX\n":
                    out_f.write("MATERIALX_NAMESPACE_BEGIN\n")
                    num_open += 1
                    seen_open = True
                    continue
                if cur_line == "{\n" and seen_open:
                    out_f.write("\n")
                    seen_open = False
                    continue
                if cur_line == "} // namespace MaterialX\n" or cur_line == "} // namespace MaterialX":
                    out_f.write("MATERIALX_NAMESPACE_END\n")
                    num_close += 1
                    continue
                    
                out_f.write(cur_line)
    if num_open == 0 or num_open != num_close:
        os.remove(cur_file+b".fixed")
        subprocess.run([git_exe, "restore", cur_file])
        print("REVERTED:", cur_file, "OPENED:", num_open, "CLOSED:", num_close)
        return
        
    os.remove(cur_file)
    os.rename(cur_file+b".fixed", cur_file)
    dr = subprocess.run([git_exe, "diff", "-U0", cur_file], capture_output=True).stdout.split(b"\n")

    # Revert the file (since we do not want clang-format noise in this commit)
    subprocess.run([git_exe, "restore", cur_file])
    
    # Traverse the diff and apply only the namespace edits:
    with open(cur_file, "rt") as in_f:
        allLines = in_f.readlines()

    num_open = 0
    num_close = 0
        
    source_line = 0
    added = []
    removed = []
    apply_edit = False
    validate = False
    for d in dr:
        if d.startswith(b"@@"):
            if apply_edit:
                ret_validate, ret_open, ret_close = apply_edits(allLines, source_line, added, removed)
                if ret_validate:
                    validate = True 
                num_open += ret_open
                num_close += ret_close
            lineno = d.split(b" ")[1].split(b",")[0]
            source_line = -int(lineno)
            added = []
            removed = []
            continue
        if d.startswith(b"-"):
            removed.append(d)
            continue
        if d.startswith(b"+"):
            added.append(d)
            if d == b'+MATERIALX_NAMESPACE_BEGIN' or d == b'+MATERIALX_NAMESPACE_END':
                apply_edit = True
            continue

    if apply_edit:
        ret_validate, ret_open, ret_close = apply_edits(allLines, source_line, added, removed)
        if ret_validate:
            validate = True 
        num_open += ret_open
        num_close += ret_close

    if num_open == 0 or num_open != num_close:
        print("COULD NOT REAPPLY DIFF:", cur_file, "OPENED:", num_open, "CLOSED:", num_close)
        return
    
    with open(cur_file+b".fixed", "wt") as out_f:
        out_f.writelines(allLines)

    os.remove(cur_file)
    os.rename(cur_file+b".fixed", cur_file)
    subprocess.run([git_exe, "add", cur_file])

    if validate:
        print("RECHECK:", cur_file)
    
    return 

os.chdir(mx_root)

# get all files mentioning "namespace MaterialX":
r = subprocess.run([git_exe, "grep", "-wl", "namespace MaterialX"], capture_output=True)
all_namespaces = set(r.stdout.split())

for cur_file in all_namespaces:
    if not cur_file.endswith(b".h") and not cur_file.endswith(b".cpp"):
        print("SKIPPING:", cur_file)
        continue
    
    # Run clang-format to make sure we get a closing namespace comment:
    subprocess.run([clf_exe, "-i", cur_file])
    
    # We can now process the file and replace the namespace:
    fix_namespaces(cur_file)
