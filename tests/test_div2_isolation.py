import json
import subprocess
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = Path(__file__).with_name("protected_div2_blobs.json")


def git_blob_sha1(path: Path) -> str:
    return subprocess.check_output(
        ["git", "hash-object", str(path)],
        cwd=ROOT,
        text=True,
    ).strip()


class Div2IsolationTests(unittest.TestCase):
    def test_protected_div2_sources_are_byte_identical_to_implementation_start(self):
        expected = json.loads(MANIFEST.read_text(encoding="utf-8"))
        actual = {
            relative_path: git_blob_sha1(ROOT / relative_path)
            for relative_path in expected
        }
        self.assertEqual(actual, expected)


if __name__ == "__main__":
    unittest.main()
