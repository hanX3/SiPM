#!/usr/bin/env python3
"""Build and exercise the readers/sorters in a temporary directory using PyROOT."""

import argparse
from pathlib import Path
import random
import re
import struct
import subprocess
import tempfile
import unittest

import ROOT

ROOT.gROOT.SetBatch(True)
REPO = Path(__file__).resolve().parents[1]
GROUPS = ("DT5730_DPP_PHA", "DT5720_DPP_PSD")
WAVE = [100 if i < 200 else 101 + (i - 200) // 10 for i in range(2400)]
T0 = 2**53
EVENTS = [(3, 7, T0 + 8), (2, 0, T0 + 10), (3, 7, T0 + 9), (2, 0, T0 + 7)]


def binary_record(board=1, channel=0, timestamp=100, size=None, wave=WAVE):
    if size is None:
        size = len(wave)
    return (struct.pack("<hhQHdHIBI", board, channel, timestamp, 50000, 661.7, 125, 0, 1, size)
            + struct.pack("<" + "h" * len(wave), *wave))


def parameters(directory):
    # Deliberately omit the final newline: the last channel must still be read.
    (directory / "SiPM_cali.dat").write_text("\n".join(f"{i} 5 2 1.25 0" for i in range(8)))
    (directory / "SiPM_ts_offset.dat").write_text("\n".join(f"{i} {10 if i == 7 else 0}" for i in range(8)))


class PipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="sipm-regression-")
        cls.base = Path(cls.temp.name)
        cls.executables = {}
        for group, program in [(g, p) for g in GROUPS for p in ("raw2root", "sort")] + [(GROUPS[0], "optimize_gate_par")]:
            output = cls.base / (group + "-" + program)
            variable = "OBJ" if program == "raw2root" else "out"
            result = subprocess.run(["make", "-B", "-C", str(REPO / group / program), f"{variable}={output}"],
                                    text=True, capture_output=True)
            if result.returncode:
                raise RuntimeError(result.stdout + result.stderr)
            cls.executables[group, program] = output

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def setUp(self):
        self.case = Path(tempfile.mkdtemp(dir=self.base))

    def run_program(self, group, program, cwd, args, ok=True, message=None):
        result = subprocess.run([str(self.executables[group, program]), *map(str, args)],
                                cwd=cwd, capture_output=True, text=True, timeout=30)
        output = result.stdout + result.stderr
        if ok:
            self.assertEqual(result.returncode, 0, output)
        else:
            self.assertGreater(result.returncode, 0, output)  # no signal/crash
        if message:
            self.assertIn(message, output)
        return result

    def prepare(self, group, payload=None):
        home = Path(tempfile.mkdtemp(dir=self.case))
        raw = home / "raw2root"
        sort = home / "sort"
        raw.mkdir()
        sort.mkdir()
        parameters(sort)
        source = home / "input.BIN"
        if payload is None:
            payload = b"".join(binary_record(*event) for event in EVENTS)
        source.write_bytes(struct.pack("<H", 0xCAEF) + payload)
        self.run_program(group, "raw2root", raw, [1, source])
        return raw, sort, source

    def test_decoding_and_sorting_values(self):
        for group in GROUPS:
            with self.subTest(group=group):
                raw, sort, _ = self.prepare(group)
                file = ROOT.TFile.Open(str(raw / "run0001.root"))
                tree = file.Get("tr_ch07")
                self.assertEqual(tree.GetEntries(), 2)
                self.assertEqual(tree.GetTitle(), group)
                self.assertEqual(tree.GetLeaf("energy_keV").GetTypeName(), "Double_t")
                tree.GetEntry(0)
                self.assertAlmostEqual(tree.energy_keV, 661.7)
                self.assertEqual(tree.energy_short, 125)
                self.assertEqual(list(tree.data), WAVE)
                file.Close()

                self.run_program(group, "sort", sort, [1])
                file = ROOT.TFile.Open(str(sort / "run0001_sort.root"))
                tree = file.Get("tr")
                self.assertEqual(tree.GetEntries(), 4)
                result = [(e.board, e.channel, e.timestamp, e.energy) for e in tree]
                expected = sorted((b, c, t + (10 if c == 7 else 0), 5 + 2 * 50000 + 1.25 * 50000**2)
                                  for b, c, t in EVENTS)
                expected.sort(key=lambda e: e[2])
                self.assertEqual(result, expected)
                definitions = dict(re.findall(r"#define\s+(QDC_\w+)\s+(\d+)",
                                              (REPO / group / "sort/set.h").read_text()))
                tree.GetEntry(0)
                self.assertEqual(tree.baseline, 100)
                self.assertEqual(tree.amplitude_max, max(WAVE) - 100)
                self.assertEqual(tree.energy_qdc, sum(x - 100 for x in WAVE))
                for gate in ("LONG", "SHORT"):
                    start, stop = (int(definitions[f"QDC_{gate}_{bound}"]) for bound in ("START", "STOP"))
                    self.assertAlmostEqual(getattr(tree, "qdc_" + gate.lower()),
                                           sum(x - 100 for x in WAVE[start:stop]) / (stop - start))
                file.Close()

    def test_truncated_and_out_of_range_records(self):
        bad = [(b"\x00", "truncated"), (binary_record()[:-1], "truncated"),
               (binary_record(channel=8), "channel"), (binary_record(channel=-1), "channel"),
               (binary_record(size=10001), "length"), (binary_record(size=0, wave=[]), "length")]
        for group in GROUPS:
            for payload, message in bad:
                with self.subTest(group=group, message=message):
                    home = Path(tempfile.mkdtemp(dir=self.case))
                    source = home / "bad.BIN"
                    source.write_bytes(struct.pack("<H", 0xCAEF) + payload)
                    self.run_program(group, "raw2root", home, [1, source], False, message)

    def test_existing_outputs_are_preserved(self):
        for group in GROUPS:
            raw, sort, source = self.prepare(group)
            before = (raw / "run0001.root").read_bytes()
            self.run_program(group, "raw2root", raw, [1, source], False, "cannot create")
            self.assertEqual((raw / "run0001.root").read_bytes(), before)
            self.run_program(group, "sort", sort, [1])
            before = (sort / "run0001_sort.root").read_bytes()
            self.run_program(group, "sort", sort, [1], False, "cannot create")
            self.assertEqual((sort / "run0001_sort.root").read_bytes(), before)

    def test_missing_and_invalid_parameters(self):
        for group in GROUPS:
            _, sort, _ = self.prepare(group)
            cali = sort / "SiPM_cali.dat"
            cali.unlink()
            self.run_program(group, "sort", sort, [1], False, "parameter file")
            for text, message in [("0 0 1 0 0", "missing channel"), ("8 0 1 0 0", "invalid"),
                                  ("0 0 1", "invalid"), ("0 0 1 0 0\n0 0 1 0 0", "duplicate")]:
                cali.write_text(text)
                self.run_program(group, "sort", sort, [1], False, message)
            parameters(sort)
            self.run_program(group, "sort", sort, [999], False, "cannot open")

    def test_short_waveforms_and_invalid_arguments(self):
        for group in GROUPS:
            _, sort, _ = self.prepare(group, binary_record(wave=[100] * 20))
            self.run_program(group, "sort", sort, [1], False, "waveform length")
            for program in ("raw2root", "sort"):
                self.run_program(group, program, sort, ["invalid"], False, "invalid nonnegative")

    def test_external_root_waveform_length(self):
        # An oversized ROOT array must be rejected BEFORE GetEntry reads its payload.
        from array import array
        home = self.case
        (home / "raw2root").mkdir()
        sort = home / "sort"
        sort.mkdir()
        parameters(sort)
        file = ROOT.TFile(str(home / "raw2root/run0001.root"), "RECREATE")
        tree = ROOT.TTree("tr_ch00", "oversized")
        values = {}
        for name, code, leaf, value in [("board", "h", "S", 0), ("channel", "h", "S", 0),
                                       ("timestamp", "q", "L", 1), ("energy_ch", "H", "s", 1),
                                       ("size", "I", "i", 10001)]:
            values[name] = array(code, [value])
            tree.Branch(name, values[name], name + "/" + leaf)
        for name in ("data", "dt"):
            values[name] = array("h", [100] * 10001)
            tree.Branch(name, values[name], name + "[size]/S")
        tree.Fill()
        tree.Write()
        file.Close()
        self.run_program(GROUPS[0], "sort", sort, [1], False, "waveform length")

    def test_gate_scan(self):
        _, sort, _ = self.prepare(GROUPS[0])
        self.run_program(GROUPS[0], "optimize_gate_par", sort, [1, 500, 2000, 400, 900])
        file = ROOT.TFile.Open(str(sort / "rootfile/run0001_lg500lg2000_sg400sg900.root"))
        tree = file.Get("tr")
        self.assertEqual(tree.GetEntries(), 4)
        tree.GetEntry(0)
        self.assertEqual(tree.board, 2)
        self.assertEqual(tree.timestamp, T0 + 7)
        file.Close()
        self.run_program(GROUPS[0], "optimize_gate_par", sort, [1, 500, 500, 400, 900], False, "gates")
        self.run_program(GROUPS[0], "optimize_gate_par", sort, [1, 0, 10001, 400, 900], False, "gates")

    def test_fom_empty_input_returns(self):
        home = self.case
        for group in GROUPS:
            file = ROOT.TFile(str(home / "run0001_sort.root"), "RECREATE")
            from array import array
            q = array("d", [0])
            tree = ROOT.TTree("tr", "empty")
            tree.Branch("qdc_long", q, "qdc_long/D")
            tree.Branch("qdc_short", q, "qdc_short/D")
            tree.Write()
            file.Close()
            macro = home / "check_empty.C"
            macro.write_text(f'#include "{REPO / group / "sort/get_fom.cpp"}"\n'
                             'void check_empty() { get_fom_single(1, 2); }\n')
            result = subprocess.run(["root", "-l", "-b", "-q", str(macro)], cwd=home,
                                    capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("empty PID histogram", result.stdout + result.stderr)

    def test_fom_known_peaks(self):
        from array import array
        rng = random.Random(20260929)
        for run, mean, sigma in [(1, -2, 0.4), (2, 2, 0.6)]:
            file = ROOT.TFile(str(self.case / f"run{run:04d}_sort.root"), "RECREATE")
            tree = ROOT.TTree("tr", "Gaussian PID reference")
            ql, qs = array("d", [1]), array("d", [0])
            tree.Branch("qdc_long", ql, "qdc_long/D")
            tree.Branch("qdc_short", qs, "qdc_short/D")
            for _ in range(20000):
                qs[0] = 1 - rng.gauss(mean, sigma)
                tree.Fill()
            tree.Write()
            file.Close()
        for group in GROUPS:
            macro = self.case / "check_peaks.C"
            macro.write_text(f'#include "{REPO / group / "sort/get_fom.cpp"}"\n'
                             'void check_peaks() { get_fom_single(1, 2); }\n')
            result = subprocess.run(["root", "-l", "-b", "-q", str(macro)], cwd=self.case,
                                    capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertNotIn("Gaussian fit failed", result.stdout + result.stderr)
            separation = float(result.stdout.strip().splitlines()[-1])
            self.assertAlmostEqual(separation, 4 / (0.4 + 0.6), delta=0.35)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=REPO)
    args, rest = parser.parse_known_args()
    REPO = args.source.resolve()
    unittest.main(argv=[__file__, *rest], verbosity=2)
