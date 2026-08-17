import glob
import re
import sys


# Every pattern captures the text in front of the version and nothing behind it,
# so that a replacement is just group one followed by the new number.
#
# The value in iraspa/CMakeLists.txt is the one the build derives everything else
# from (CPack, the macOS bundle), so it doubles as the source of truth for what
# the current version is.
CMAKE_FILE = "iraspa/CMakeLists.txt"
CMAKE_PATTERN = r'(^[ \t]*set\(IRASPA_VERSION[ \t]+")\d+\.\d+\.\d+(?=")'


def version_targets():
    targets = [
        (CMAKE_FILE, CMAKE_PATTERN),
        # "Version 2.3.2 (build 142)" in the about box; the build number is a
        # separate counter and is left alone.
        ("iraspa/iraspagui/aboutdialog.ui", r"(Version[ \t]+)\d+\.\d+\.\d+"),
        ("iraspa/snapcraft.yaml", r"(^version:[ \t]*)\d+\.\d+\.\d+"),
        ("arch/PKGBUILD", r"(^pkgver=)\d+\.\d+\.\d+"),
        ("arch/from-github/PKGBUILD", r"(^pkgver=)\d+\.\d+\.\d+"),
        # Only the topmost entry, which is the one that carries the distribution
        # suffix the readme describes. Older entries are history.
        ("debian/changelog", r"(\Airaspa \()\d+\.\d+\.\d+"),
    ]
    for spec in sorted(glob.glob("specs/*/iraspa.spec")):
        targets.append((spec, r"(^Version:[ \t]*)\d+\.\d+\.\d+"))
    return targets


def bump_version(file_path, pattern, new_version):
    with open(file_path, "r") as file:
        content = file.read()

    new_content, count = re.subn(pattern, rf"\g<1>{new_version}", content, flags=re.MULTILINE)

    if count == 0:
        raise ValueError(f"Version pattern not found in {file_path}")

    with open(file_path, "w") as file:
        file.write(new_content)

    return count


def read_current_version(file_path, pattern):
    with open(file_path, "r") as file:
        content = file.read()

    match = re.search(pattern, content, flags=re.MULTILINE)
    if match is None:
        raise ValueError(f"Version pattern not found in {file_path}")

    version = re.search(r"\d+\.\d+\.\d+", match.group(0))
    return version.group(0)


def increment_version(current_version, part):
    major, minor, patch = map(int, current_version.split("."))
    if part == "major":
        major += 1
        minor = 0
        patch = 0
    elif part == "minor":
        minor += 1
        patch = 0
    elif part == "patch":
        patch += 1
    else:
        raise ValueError("Invalid part to increment. Choose 'major', 'minor', or 'patch'.")
    return f"{major}.{minor}.{patch}"


def main(new_version_or_part):
    current_version = read_current_version(CMAKE_FILE, CMAKE_PATTERN)

    if new_version_or_part in ["major", "minor", "patch"]:
        new_version = increment_version(current_version, new_version_or_part)
    else:
        new_version = new_version_or_part

    print(f"{current_version} -> {new_version}")
    for file_path, pattern in version_targets():
        count = bump_version(file_path, pattern, new_version)
        print(f"  {file_path} ({count})")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python bump_version.py <new_version_or_part>")
    elif (sys.argv[1] not in ["major", "minor", "patch"]) and (not bool(re.match(r"^\d+\.\d+\.\d+$", sys.argv[1]))):
        print("Usage: python bump_version.py <new_version_or_part>")
    else:
        main(sys.argv[1])
