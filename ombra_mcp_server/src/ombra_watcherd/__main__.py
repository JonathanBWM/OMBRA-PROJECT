#!/usr/bin/env python3
"""
CLI entry point for ombra-watcherd

Usage:
    ombra-watcherd install   # Install launchd plist, start service
    ombra-watcherd start     # Start daemon
    ombra-watcherd stop      # Stop daemon
    ombra-watcherd status    # Show running state
    ombra-watcherd uninstall # Remove launchd plist
    ombra-watcherd run       # Run in foreground (for debugging)
"""

import sys
import os
import argparse
from pathlib import Path

from .launchd import LaunchdManager, WATCH_PATH as DEFAULT_WATCH_PATH
from .daemon import OmbraWatcherd


def main():
    parser = argparse.ArgumentParser(
        description="OmbraWatcherd - Living codebase intelligence daemon",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )

    subparsers = parser.add_subparsers(dest="command", help="Commands")

    # install
    install_parser = subparsers.add_parser("install", help="Install and start launchd service")

    # uninstall
    uninstall_parser = subparsers.add_parser("uninstall", help="Stop and remove launchd service")

    # start
    start_parser = subparsers.add_parser("start", help="Start the daemon")

    # stop
    stop_parser = subparsers.add_parser("stop", help="Stop the daemon")

    # status
    status_parser = subparsers.add_parser("status", help="Show daemon status")

    # run (foreground)
    run_parser = subparsers.add_parser("run", help="Run daemon in foreground")
    run_parser.add_argument(
        "--watch-path",
        type=Path,
        default=DEFAULT_WATCH_PATH,
        help="Path to watch (default: auto-detected or OMBRA_PROJECT_ROOT env var)"
    )
    run_parser.add_argument(
        "--db-path",
        type=Path,
        default=None,
        help="Path to project_brain.db (default: in MCP data dir)"
    )

    # scan (one-shot analysis)
    scan_parser = subparsers.add_parser("scan", help="Run one-shot analysis")
    scan_parser.add_argument(
        "--path",
        type=Path,
        default=DEFAULT_WATCH_PATH,
        help="Path to scan (default: auto-detected or OMBRA_PROJECT_ROOT env var)"
    )

    args = parser.parse_args()

    if args.command is None:
        parser.print_help()
        sys.exit(1)

    manager = LaunchdManager()

    if args.command == "install":
        if manager.install():
            print("Installed and started ombra-watcherd")
            sys.exit(0)
        else:
            print("Failed to install", file=sys.stderr)
            sys.exit(1)

    elif args.command == "uninstall":
        if manager.uninstall():
            print("Uninstalled ombra-watcherd")
            sys.exit(0)
        else:
            print("Failed to uninstall", file=sys.stderr)
            sys.exit(1)

    elif args.command == "start":
        if manager.start():
            print("Started ombra-watcherd")
            sys.exit(0)
        else:
            print("Failed to start", file=sys.stderr)
            sys.exit(1)

    elif args.command == "stop":
        if manager.stop():
            print("Stopped ombra-watcherd")
            sys.exit(0)
        else:
            print("Failed to stop", file=sys.stderr)
            sys.exit(1)

    elif args.command == "status":
        status = manager.status()
        print(f"Status: {status['state']}")
        if status.get('pid'):
            print(f"PID: {status['pid']}")
        if status.get('last_scan'):
            print(f"Last scan: {status['last_scan']}")
        if status.get('findings_count') is not None:
            print(f"Active findings: {status['findings_count']}")
        sys.exit(0)

    elif args.command == "run":
        # Run in foreground
        daemon = OmbraWatcherd(
            watch_path=args.watch_path,
            db_path=args.db_path
        )
        try:
            daemon.run()
        except KeyboardInterrupt:
            print("\nShutting down...")
            daemon.stop()

    elif args.command == "scan":
        # One-shot analysis
        daemon = OmbraWatcherd(watch_path=args.path)
        results = daemon.scan_all()
        print(f"Scanned {results['files_scanned']} files")
        print(f"Found {results['findings_count']} findings")
        for severity in ['critical', 'warning', 'info']:
            count = results.get(f'{severity}_count', 0)
            if count > 0:
                print(f"  {severity}: {count}")


if __name__ == "__main__":
    main()
