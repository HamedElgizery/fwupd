// Copyright 2025 Hamed Elgizery <email>
// SPDX-License-Identifier: LGPL-2.1-or-later

#[derive(New, ValidateStream, ParseStream, Default)]
#[repr(C, packed)]
struct FuStructCrosHammer {
    signature: u8 == 0xDE,
    address: u16le,
}

#[derive(ToString)]
enum FuCrosHammerStatus {
    Unknown,
    Failed,
}
