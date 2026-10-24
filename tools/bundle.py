#!/usr/bin/env python3
"""Inline project headers and remove selected local-only blocks for submission."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

LOCAL_INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"\s*(?://.*)?$')
PRAGMA_ONCE = re.compile(r"^\s*#\s*pragma\s+once\s*(?://.*)?$")
IFDEF = re.compile(r"^\s*#\s*ifdef\s+([A-Za-z_]\w*)\s*(?://.*)?$")
CONDITIONAL_START = re.compile(r"^\s*#\s*(?:if|ifdef|ifndef)\b")
CONDITIONAL_END = re.compile(r"^\s*#\s*endif\b")
CONDITIONAL_ALTERNATIVE = re.compile(r"^\s*#\s*(?:else|elif)\b")


class BundleError(RuntimeError):
    pass


class Bundler:
    def __init__(self, include_dirs: list[Path], strip_ifdefs: list[str] | None = None) -> None:
        self.include_dirs = [path.resolve() for path in include_dirs]
        self.strip_ifdefs = set(strip_ifdefs or [])
        self.included: set[Path] = set()

    def source_lines(self, lines: list[str], path: Path) -> list[tuple[int, str]]:
        output: list[tuple[int, str]] = []
        skipped_depth = 0
        skipped_macro = ""
        skipped_line = 0

        for line_number, line in enumerate(lines, start=1):
            if skipped_depth == 0:
                match = IFDEF.match(line)
                if match is not None and match.group(1) in self.strip_ifdefs:
                    skipped_depth = 1
                    skipped_macro = match.group(1)
                    skipped_line = line_number
                    continue
                output.append((line_number, line))
                continue

            if CONDITIONAL_START.match(line):
                skipped_depth += 1
            elif CONDITIONAL_END.match(line):
                skipped_depth -= 1
            elif skipped_depth == 1 and CONDITIONAL_ALTERNATIVE.match(line):
                raise BundleError(
                    f"{path}:{line_number}: cannot strip #ifdef {skipped_macro} containing #else or #elif"
                )

        if skipped_depth != 0:
            raise BundleError(f"{path}:{skipped_line}: unterminated #ifdef {skipped_macro}")
        return output

    def resolve_include(self, name: str, including_file: Path) -> Path | None:
        candidates = [including_file.parent / name]
        candidates.extend(root / name for root in self.include_dirs)
        for candidate in candidates:
            if candidate.is_file():
                return candidate.resolve()
        return None

    def display_path(self, path: Path) -> str:
        for root in self.include_dirs:
            try:
                return path.relative_to(root).as_posix()
            except ValueError:
                pass
        return path.name

    def expand(self, path: Path, *, is_root: bool = False) -> list[str]:
        path = path.resolve()
        if not is_root and path in self.included:
            return []
        if not is_root:
            self.included.add(path)

        try:
            lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
        except OSError as error:
            raise BundleError(f"cannot read {path}: {error}") from error

        output: list[str] = []
        if not is_root:
            output.append(f"// ===== BEGIN INLINE: {self.display_path(path)} =====\n")

        for line_number, line in self.source_lines(lines, path):
            if PRAGMA_ONCE.match(line):
                continue

            match = LOCAL_INCLUDE.match(line)
            if match is None:
                output.append(line)
                continue

            include_name = match.group(1)
            included_path = self.resolve_include(include_name, path)
            if included_path is None:
                raise BundleError(
                    f'{path}:{line_number}: cannot resolve local include "{include_name}"'
                )
            output.extend(self.expand(included_path))

        if not is_root:
            if output and not output[-1].endswith("\n"):
                output[-1] += "\n"
            output.append(f"// ===== END INLINE: {self.display_path(path)} =====\n")
        return output


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True, help="solution .cpp file")
    parser.add_argument(
        "--include-dir",
        type=Path,
        action="append",
        default=[],
        help="root used to resolve quoted includes; repeat when needed",
    )
    parser.add_argument("--output", type=Path, required=True, help="bundled .cpp path")
    parser.add_argument(
        "--strip-ifdef",
        action="append",
        default=[],
        metavar="MACRO",
        help="remove standalone #ifdef MACRO blocks from the bundle; repeat when needed",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    source = args.source.resolve()
    output = args.output.resolve()
    if source == output:
        raise BundleError("source and output must be different files")

    bundler = Bundler(args.include_dir, args.strip_ifdef)
    bundled = "".join(bundler.expand(source, is_root=True))
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(bundled, encoding="utf-8")
    print(f"Bundled {source} -> {output} ({len(bundler.included)} local headers)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except BundleError as error:
        raise SystemExit(f"bundle error: {error}") from error

