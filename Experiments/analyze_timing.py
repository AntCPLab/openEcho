#!/usr/bin/env python3
"""
Script to analyze timing and instruction statistics from MNIST experiment results.
Computes average timing and instruction counts across parties for RSS and Echo Online protocols.
"""

import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple
from collections import defaultdict


def parse_timing_data(file_path: str) -> Dict[str, Dict[str, Tuple[float, float]]]:
    """
    Parse timing data from markdown file.
    
    Returns:
        Dict structure: {protocol: {party: (total_time, compute_time)}}
    """
    with open(file_path, 'r') as f:
        content = f.read()
    
    results = {}
    
    # Split by protocol sections (## Echo Online, ## RSS, etc.)
    protocol_sections = re.split(r'^## ', content, flags=re.MULTILINE)
    
    for section in protocol_sections:
        if not section.strip():
            continue
            
        lines = section.split('\n')
        protocol_name = lines[0].strip()
        
        if protocol_name not in results:
            results[protocol_name] = {}
        
        # Find party sections (P0:, P1:, P2:)
        current_party = None
        for line in lines:
            # Detect party marker
            party_match = re.match(r'^P(\d):', line)
            if party_match:
                current_party = f"P{party_match.group(1)}"
                continue
            
            # Extract timing data
            if current_party:
                # Extract Time = X seconds
                time_match = re.search(r'^Time = ([\d.]+) seconds', line)
                if time_match:
                    total_time = float(time_match.group(1))
                    
                # Extract Time1 = X seconds (compute time)
                time1_match = re.search(r'^Time1 = ([\d.]+) seconds', line)
                if time1_match:
                    compute_time = float(time1_match.group(1))
                    results[protocol_name][current_party] = (total_time, compute_time)
    
    return results


def parse_instruction_stats(file_path: str) -> Dict[str, Dict[str, Dict[str, int]]]:
    """
    Parse instruction statistics from markdown file.
    
    Returns:
        Dict structure: {protocol: {party: {instruction: count}}}
    """
    with open(file_path, 'r') as f:
        content = f.read()
    
    results = {}
    
    # Split by protocol sections (## Echo Online, ## RSS, etc.)
    protocol_sections = re.split(r'^## ', content, flags=re.MULTILINE)
    
    for section in protocol_sections:
        if not section.strip():
            continue
            
        lines = section.split('\n')
        protocol_name = lines[0].strip()
        
        if protocol_name not in results:
            results[protocol_name] = {}
        
        # Find party sections (P0:, P1:, P2:)
        current_party = None
        in_instruction_stats = False
        
        for line in lines:
            # Detect party marker
            party_match = re.match(r'^P(\d):', line)
            if party_match:
                current_party = f"P{party_match.group(1)}"
                in_instruction_stats = False
                results[protocol_name][current_party] = {}
                continue
            
            # Detect instruction statistics section
            if current_party and 'Instruction statistics:' in line:
                in_instruction_stats = True
                continue
            
            # Parse instruction lines
            if current_party and in_instruction_stats:
                # Match lines like: "        USE_INP        41"
                instr_match = re.match(r'^\s+([A-Z_]+)\s+([\d]+)', line)
                if instr_match:
                    instruction = instr_match.group(1)
                    count = int(instr_match.group(2))
                    results[protocol_name][current_party][instruction] = count
                elif line.strip() and not line.strip().startswith('```'):
                    # End of instruction stats section
                    if 'This program might benefit' in line or line.strip() == '```':
                        in_instruction_stats = False
    
    return results


def compute_averages(data: Dict[str, Dict[str, Tuple[float, float]]]) -> Dict[str, Tuple[float, float]]:
    """
    Compute average timings for each protocol.
    
    For RSS: average of all 3 parties (P0, P1, P2)
    For Echo Online: average of only P1 and P2 (ignore P0)
    """
    averages = {}
    
    for protocol, parties in data.items():
        if 'RSS' in protocol:
            # Average all three parties
            party_list = ['P0', 'P1', 'P2']
        elif 'Echo' in protocol:
            # Only P1 and P2
            party_list = ['P1', 'P2']
        else:
            # Default: use all available parties
            party_list = list(parties.keys())
        
        # Filter to parties that exist
        valid_parties = [p for p in party_list if p in parties]
        
        if not valid_parties:
            continue
        
        # Compute averages
        avg_total = sum(parties[p][0] for p in valid_parties) / len(valid_parties)
        avg_compute = sum(parties[p][1] for p in valid_parties) / len(valid_parties)
        
        averages[protocol] = (avg_total, avg_compute, valid_parties)
    
    return averages


def compute_instruction_averages(data: Dict[str, Dict[str, Dict[str, int]]]) -> Dict[str, Tuple[Dict[str, float], List[str]]]:
    """
    Compute average instruction counts for each protocol.
    
    For RSS: average of all 3 parties (P0, P1, P2)
    For Echo Online: average of only P1 and P2 (ignore P0)
    """
    averages = {}
    
    for protocol, parties in data.items():
        if 'RSS' in protocol:
            # Average all three parties
            party_list = ['P0', 'P1', 'P2']
        elif 'Echo' in protocol:
            # Only P1 and P2
            party_list = ['P1', 'P2']
        else:
            # Default: use all available parties
            party_list = list(parties.keys())
        
        # Filter to parties that exist
        valid_parties = [p for p in party_list if p in parties]
        
        if not valid_parties:
            continue
        
        # Get all instruction types
        all_instructions = set()
        for party in valid_parties:
            all_instructions.update(parties[party].keys())
        
        # Compute averages for each instruction
        avg_instructions = {}
        for instruction in all_instructions:
            total = sum(parties[p].get(instruction, 0) for p in valid_parties)
            avg_instructions[instruction] = total / len(valid_parties)
        
        averages[protocol] = (avg_instructions, valid_parties)
    
    return averages


def print_results(file_name: str, data: Dict[str, Dict[str, Tuple[float, float]]]):
    """Print detailed results for a file."""
    print(f"\n{'='*80}")
    print(f"Results from: {file_name}")
    print(f"{'='*80}")
    
    for protocol in sorted(data.keys()):
        print(f"\n{protocol}")
        print("-" * 80)
        
        parties = data[protocol]
        for party in sorted(parties.keys()):
            total_time, compute_time = parties[party]
            print(f"  {party}: Total={total_time:7.4f}s, Compute={compute_time:7.4f}s")
        
        # Compute and show average
        averages = compute_averages({protocol: parties})
        if protocol in averages:
            avg_total, avg_compute, used_parties = averages[protocol]
            parties_str = ', '.join(used_parties)
            print(f"  {'Avg'} ({parties_str}): Total={avg_total:7.4f}s, Compute={avg_compute:7.4f}s")


def print_protocol_comparison(file_name: str, data: Dict[str, Dict[str, Tuple[float, float]]]):
    """Compare RSS vs Echo Online within the same dataset."""
    print(f"\n\n{'='*80}")
    print(f"PROTOCOL COMPARISON: {file_name}")
    print(f"{'='*80}\n")
    
    avgs = compute_averages(data)
    
    # Find Echo and RSS protocols
    echo_key = None
    rss_key = None
    
    for protocol in avgs.keys():
        if 'Echo' in protocol:
            echo_key = protocol
        elif 'RSS' in protocol:
            rss_key = protocol
    
    if not echo_key or not rss_key:
        print("Could not find both Echo and RSS protocols in data.")
        return
    
    echo_total, echo_compute, echo_parties = avgs[echo_key]
    rss_total, rss_compute, rss_parties = avgs[rss_key]
    
    print(f"{'Protocol':<20} | {'Total Time':<15} | {'Compute Time':<15} | {'Parties Averaged'}")
    print("-" * 80)
    print(f"{echo_key:<20} | {echo_total:>13.4f}s | {echo_compute:>13.4f}s | {', '.join(echo_parties)}")
    print(f"{rss_key:<20} | {rss_total:>13.4f}s | {rss_compute:>13.4f}s | {', '.join(rss_parties)}")
    print("-" * 80)
    
    # Calculate speedup
    time_diff = rss_total - echo_total
    time_pct = (time_diff / echo_total) * 100
    
    if time_diff > 0:
        print(f"\n✓ Echo is FASTER by {abs(time_diff):.4f}s ({abs(time_pct):.1f}%)")
    else:
        print(f"\n✓ RSS is FASTER by {abs(time_diff):.4f}s ({abs(time_pct):.1f}%)")


def print_instruction_comparison(file_name: str, instr_data: Dict[str, Dict[str, Dict[str, int]]]):
    """Compare instruction statistics between RSS and Echo Online within the same dataset."""
    print(f"\n\n{'='*80}")
    print(f"INSTRUCTION STATISTICS COMPARISON: {file_name}")
    print(f"{'='*80}\n")
    
    avgs = compute_instruction_averages(instr_data)
    
    # Find Echo and RSS protocols
    echo_key = None
    rss_key = None
    
    for protocol in avgs.keys():
        if 'Echo' in protocol:
            echo_key = protocol
        elif 'RSS' in protocol:
            rss_key = protocol
    
    if not echo_key or not rss_key:
        print("Could not find both Echo and RSS protocols in data.")
        return
    
    echo_instructions, echo_parties = avgs[echo_key]
    rss_instructions, rss_parties = avgs[rss_key]
    
    print(f"Comparing: {echo_key} ({','.join(echo_parties)}) vs {rss_key} ({','.join(rss_parties)})")
    print("-" * 100)
    
    # Get all instruction types
    all_instructions = sorted(set(echo_instructions.keys()) | set(rss_instructions.keys()))
    
    # Find instructions with significant differences
    significant_diffs = []
    for instr in all_instructions:
        echo_count = echo_instructions.get(instr, 0)
        rss_count = rss_instructions.get(instr, 0)
        diff = rss_count - echo_count
        
        if echo_count > 0:
            pct_diff = (diff / echo_count) * 100
        else:
            pct_diff = float('inf') if rss_count > 0 else 0
        
        # Significant if difference is > 10% and > 1 million, or > 1 billion absolute
        if (abs(pct_diff) > 10 and abs(diff) > 1_000_000) or abs(diff) > 1_000_000_000:
            significant_diffs.append((instr, echo_count, rss_count, diff, pct_diff))
    
    # Sort by absolute difference (descending)
    significant_diffs.sort(key=lambda x: abs(x[3]), reverse=True)
    
    if significant_diffs:
        print(f"\n{'Instruction':<20} | {'Echo Online':<20} | {'RSS':<20} | {'Difference (RSS - Echo)'}")
        print("-" * 100)
        
        for instr, echo_count, rss_count, diff, pct_diff in significant_diffs[:30]:  # Top 30
            echo_str = f"{echo_count:,.0f}"
            rss_str = f"{rss_count:,.0f}"
            diff_str = f"{diff:+,.0f}"
            
            if abs(pct_diff) < 1000:
                pct_str = f"({pct_diff:+.1f}%)"
            else:
                pct_str = "(RSS only)" if echo_count == 0 else "(Echo only)"
            
            print(f"{instr:<20} | {echo_str:>20} | {rss_str:>20} | {diff_str:>15} {pct_str}")
    else:
        print("No significant differences found.")
    
    # Print total instruction counts
    echo_total = sum(echo_instructions.values())
    rss_total = sum(rss_instructions.values())
    total_diff = rss_total - echo_total
    total_pct = (total_diff / echo_total) * 100 if echo_total > 0 else 0
    
    print("-" * 100)
    print(f"{'TOTAL':<20} | {echo_total:>20,.0f} | {rss_total:>20,.0f} | {total_diff:>15,.0f} ({total_pct:+.1f}%)")


def main():
    # Define file paths
    script_dir = Path(__file__).parent
    file_a = script_dir / "debug-mnistA.md"
    file_b = script_dir / "debug-mnistB.md"
    file_c = script_dir / "debug-resnet50.md"
    
    # Check if files exist
    if not file_a.exists():
        print(f"Error: {file_a} not found!")
        sys.exit(1)
    if not file_b.exists():
        print(f"Error: {file_b} not found!")
        sys.exit(1)
    if not file_c.exists():
        print(f"Error: {file_c} not found!")
        sys.exit(1)
    
    # Parse both files
    print("Parsing timing data...")
    data_a = parse_timing_data(str(file_a))
    data_b = parse_timing_data(str(file_b))
    data_c = parse_timing_data(str(file_c))
    print("Parsing instruction statistics...")
    instr_a = parse_instruction_stats(str(file_a))
    instr_b = parse_instruction_stats(str(file_b))
    instr_c = parse_instruction_stats(str(file_c))
    # Print detailed results for dataset A
    print_results("debug-mnistA.md", data_a)
    print_protocol_comparison("debug-mnistA.md", data_a)
    print_instruction_comparison("debug-mnistA.md", instr_a)
    
    # Print detailed results for dataset B
    print_results("debug-mnistB.md", data_b)
    print_protocol_comparison("debug-mnistB.md", data_b)
    print_instruction_comparison("debug-mnistB.md", instr_b)
    
    # Print detailed results for dataset C

    print("\n" + "="*80)
    print("Analysis complete!")
    print("="*80)


if __name__ == "__main__":
    main()
