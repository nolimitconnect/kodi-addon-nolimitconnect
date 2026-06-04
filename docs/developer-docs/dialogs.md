---
title: Dialogs
---

# Kodi Dialog Specifications

This page defines core dialogs that should be implemented and documented for the Kodi add-on GUI layer.

---

## Dialog: Network Setup

### Purpose
Collect and validate the local node's network identity and bootstrap endpoints during first run (or when the user reconfigures networking).

### Recommended Kodi UI
- Container dialog: `CGUIDialog`
- Inputs: `CGUIEditControl`
- Actions: `CGUIButtonControl`
- Validation notices: `CGUIDialogKaiToast` or inline label

### Fields
| Field | Type | Required | Notes |
|---|---|---|---|
| Display Name | text | Yes | Public user name announced to peers |
| Network Host URL | text | Yes | Default: `nolimitconnect.net` |
| Connection Test Host | text | Yes | Can default from Network Host config |
| TCP Listen Port | integer | Yes | Validate range and availability |
| Auto-connect on startup | boolean | No | If enabled, sign-on starts with add-on |

### Actions
| Action | Behavior |
|---|---|
| Test Connectivity | Runs connection test host flow and reports reachability |
| Save | Persists settings to addon profile and closes dialog |
| Cancel | Closes without writing changes |
| Reset Defaults | Restores `nolimitconnect.net` defaults |

### Engine/Bridge Integration
- GUI -> engine via `IFromGui` wrapper:
  - update user settings
  - trigger connectivity test
- Engine -> GUI via `IToGui` wrapper:
  - network state updates
  - test result callback

### Validation Rules
- Display name cannot be empty.
- Host fields must be valid host or URL strings.
- Port must be numeric and in allowed range.
- Save is disabled until required fields are valid.

---

## Dialog: Plugin Permissions

### Purpose
Allow users to configure per-plugin permission thresholds using NLC levels: Ignore, Guest, Friend, Admin.

### Recommended Kodi UI
- Container dialog: `CGUIDialog`
- Plugin list: `CGUIListContainer`
- Permission selector: `CGUISpinControlEx` or list-based picker

### Plugin Rows
Document each currently exposed plugin slot as a configurable row:

| Slot | Plugin | Permission Threshold |
|---|---|---|
| 8 | Messenger | Ignore / Guest / Friend / Admin |
| 9 | PushToTalk | Ignore / Guest / Friend / Admin |
| 10 | PersonFileXfer | Ignore / Guest / Friend / Admin |
| 11 | CamServer | Ignore / Guest / Friend / Admin |
| 12 | FileShareServer | Ignore / Guest / Friend / Admin |
| 15 | VideoChat | Ignore / Guest / Friend / Admin |
| 16 | VoicePhone | Ignore / Guest / Friend / Admin |

### Actions
| Action | Behavior |
|---|---|
| Apply | Writes permission thresholds for all changed plugin slots |
| Restore Recommended | Resets to project default thresholds |
| Cancel | Discards edits |

### Engine/Bridge Integration
- GUI -> engine via `IFromGui` wrapper:
  - set plugin permission level by slot
- Engine -> GUI via `IToGui` wrapper:
  - optional confirmation event or refresh after apply

### Validation Rules
- Every listed plugin slot must have exactly one selected threshold.
- Changes are applied atomically when possible (all-or-none).

---

## Dialog: Join Host

### Purpose
Provide a direct workflow to join a specific host/session target rather than only auto-joining RandomConnect.

### Recommended Kodi UI
- Container dialog: `CGUIDialog`
- Host input/search: `CGUIEditControl` + `CGUIButtonControl`
- Results list (optional): `CGUIListContainer`

### Fields
| Field | Type | Required | Notes |
|---|---|---|---|
| Host Name or Host URL | text | Yes | Accept direct host name, URL, or identifier |
| Plugin Target | enum | No | Optional pre-select (Messenger/VideoChat/etc.) |
| Connect Timeout (sec) | integer | No | Optional override |

### Actions
| Action | Behavior |
|---|---|
| Resolve | Queries directory/host info before join |
| Join | Initiates session join to selected host |
| Cancel | Closes dialog |

### Engine/Bridge Integration
- GUI -> engine via `IFromGui` wrapper:
  - resolve host
  - join host / start plugin session
- Engine -> GUI via `IToGui` wrapper:
  - resolve result
  - join success/failure status
  - session started event (opens target plugin dialog)

### Validation Rules
- Host field is required for Resolve/Join.
- Join is disabled until host resolves or direct join policy permits bypass.
- Surface actionable error text for timeout, not-found, and permission-denied responses.

---

## Notes for Implementation

- Keep all dialog-triggered engine calls marshalled through the existing bridge layer.
- Ensure GUI updates occur on Kodi main thread.
- Reuse shared validation helpers so first-run setup and settings screens stay consistent.
