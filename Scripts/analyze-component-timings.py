#!/usr/bin/env python3
"""
Analyze component timing output from ABY3 and Echo protocols.
Generates comparison tables for performance analysis.

Usage:
  ./Scripts/analyze-component-timings.py aby3.log echo.log
  
Or pipe directly:
  ./replicated-ring-party.x ... -o component_timing 2>&1 | tee aby3.log
  ./echo-party.x ... -o component_timing 2>&1 | tee echo.log
  ./Scripts/analyze-component-timings.py aby3.log echo.log
"""

import sys
import re
from collections import defaultdict

def parse_timing_report(log_content, protocol_name):
    """Parse component timing report from log output."""
    timings = {}
    
    # Match lines like "Component Name:    123.4567s"
    pattern = r'^([A-Za-z ]+):\s+([0-9.]+)s'
    
    # Also match total lines
    total_pattern = r'^(?:Protocol Total|-> \w+ Total):\s+([0-9.]+)s'
    
    in_timing_section = False
    current_section = None
    
    for line in log_content.split('\n'):
        line = line.strip()
        
        # Check if we're entering a timing section
        if 'Component Timing' in line or '=== ABY3' in line or '=== Echo' in line:
            in_timing_section = True
            continue
        
        # Check for end of section
        if in_timing_section and line.startswith('===='):
            in_timing_section = False
            continue
        
        if not in_timing_section:
            continue
        
        # Parse timing lines
        match = re.match(pattern, line)
        if match:
            component = match.group(1).strip()
            time_val = float(match.group(2))
            timings[component] = time_val
        
        # Parse total lines
        total_match = re.search(r'-> ([A-Za-z ]+) Total:\s+([0-9.]+)s', line)
        if total_match:
            component = f"{total_match.group(1)} Total"
            time_val = float(total_match.group(2))
            timings[component] = time_val
        
        # Parse Protocol Total
        proto_total = re.match(r'^Protocol Total:\s+([0-9.]+)s', line)
        if proto_total:
            timings['Protocol Total'] = float(proto_total.group(1))
    
    return timings

def generate_comparison_table(aby3_timings, echo_timings, total_time_gap=None):
    """Generate a comparison table from the timing data."""
    
    # Get all components from both
    all_components = set(aby3_timings.keys()) | set(echo_timings.keys())
    
    # Define display order (higher priority first)
    priority = [
        'Arith Mul Total', 'Arith Mul Prepare', 'Arith Mul Exchange', 'Arith Mul Finalize',
        'Dotprod Total', 'Dotprod Prepare', 'Dotprod Next', 'Dotprod Exchange', 'Dotprod Finalize',
        'Binary Total', 'Binary Prepare', 'Binary Exchange', 'Binary Finalize',
        'Truncation',
        'Prep Total', 'Prep Read', 'Prep Write',
        'Protocol Total'
    ]
    
    # Sort components by priority
    def get_priority(comp):
        try:
            return priority.index(comp)
        except ValueError:
            return len(priority)
    
    sorted_components = sorted(all_components, key=get_priority)
    
    # Calculate total gap if not provided
    if total_time_gap is None:
        aby3_total = aby3_timings.get('Protocol Total', sum(aby3_timings.values()))
        echo_total = echo_timings.get('Protocol Total', sum(echo_timings.values()))
        total_time_gap = aby3_total - echo_total
    
    # Print header
    print("\n" + "=" * 80)
    print("COMPONENT-BY-COMPONENT PERFORMANCE COMPARISON")
    print("=" * 80)
    print()
    
    # Print table header
    print(f"{'Component':<25} {'ABY3 (s)':>12} {'Echo (s)':>12} {'Delta (s)':>12} {'Gap %':>10}")
    print("-" * 75)
    
    # Print each component
    for comp in sorted_components:
        aby3_val = aby3_timings.get(comp, 0.0)
        echo_val = echo_timings.get(comp, 0.0)
        delta = aby3_val - echo_val
        
        if total_time_gap > 0:
            gap_pct = (delta / total_time_gap * 100) if total_time_gap != 0 else 0
        else:
            gap_pct = 0
        
        # Format the row
        if comp.endswith('Total'):
            print(f"  {comp:<23} {aby3_val:>12.4f} {echo_val:>12.4f} {delta:>12.4f} {gap_pct:>9.1f}%")
        else:
            print(f"{comp:<25} {aby3_val:>12.4f} {echo_val:>12.4f} {delta:>12.4f} {gap_pct:>9.1f}%")
    
    print("-" * 75)
    
    # Summary
    aby3_total = aby3_timings.get('Protocol Total', sum(aby3_timings.values()))
    echo_total = echo_timings.get('Protocol Total', sum(echo_timings.values()))
    
    print()
    print(f"ABY3 Protocol Total: {aby3_total:.4f}s")
    print(f"Echo Protocol Total: {echo_total:.4f}s")
    print(f"Total Gap: {total_time_gap:.4f}s")
    if aby3_total > 0:
        print(f"Echo is {(aby3_total/echo_total - 1)*100:.1f}% faster" if echo_total < aby3_total 
              else f"ABY3 is {(echo_total/aby3_total - 1)*100:.1f}% faster")
    print()
    print("=" * 80)

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        print("\nUsage: analyze-component-timings.py <aby3.log> <echo.log>")
        sys.exit(1)
    
    aby3_log_file = sys.argv[1]
    echo_log_file = sys.argv[2]
    
    try:
        with open(aby3_log_file, 'r') as f:
            aby3_content = f.read()
    except FileNotFoundError:
        print(f"Error: ABY3 log file not found: {aby3_log_file}")
        sys.exit(1)
    
    try:
        with open(echo_log_file, 'r') as f:
            echo_content = f.read()
    except FileNotFoundError:
        print(f"Error: Echo log file not found: {echo_log_file}")
        sys.exit(1)
    
    aby3_timings = parse_timing_report(aby3_content, "ABY3")
    echo_timings = parse_timing_report(echo_content, "Echo")
    
    if not aby3_timings and not echo_timings:
        print("No timing data found in log files.")
        print("Make sure to run with '-o component_timing' option.")
        sys.exit(1)
    
    # Extract total time from benchmark output if available
    total_time_pattern = r'Time1 = ([0-9.]+) seconds'
    aby3_total_match = re.search(total_time_pattern, aby3_content)
    echo_total_match = re.search(total_time_pattern, echo_content)
    
    total_gap = None
    if aby3_total_match and echo_total_match:
        aby3_time1 = float(aby3_total_match.group(1))
        echo_time1 = float(echo_total_match.group(1))
        total_gap = aby3_time1 - echo_time1
        print(f"\nExtracted from benchmark output:")
        print(f"  ABY3 Time1: {aby3_time1:.4f}s")
        print(f"  Echo Time1: {echo_time1:.4f}s")
        print(f"  Gap: {total_gap:.4f}s ({total_gap/aby3_time1*100:.1f}% of ABY3 time)")
    
    generate_comparison_table(aby3_timings, echo_timings, total_gap)

if __name__ == '__main__':
    main()
