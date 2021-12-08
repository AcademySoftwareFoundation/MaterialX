# Fixup code namespaces to be customizable:
# As run for MATX-341 update phase

import subprocess, os

mx_root = r"."
clf_exe = r"clang-format"
git_exe = r"git"

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
    if num_open > 0 and num_open == num_close:
        os.remove(cur_file)
        os.rename(cur_file+b".fixed", cur_file)
        subprocess.run([git_exe, "add", cur_file])
    else:
        os.remove(cur_file+b".fixed")
        subprocess.run([git_exe, "restore", cur_file])
        print("REVERTED:", cur_file, "OPENED:", num_open, "CLOSED:", num_close)
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