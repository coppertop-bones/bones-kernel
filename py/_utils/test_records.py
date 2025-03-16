

# Python imports
import datetime as dt
from typing import Union

# symmetry.common imports
from bones.core.sentinels import Missing

# local imports
from type_lang._utils.immutable_records import newImmutableRecordType, immutable_record, ImmutableRecord, \
    joinImmutableRecordTypes, fitsWithin
from type_lang._utils.testing import assertRaises


class XeHandle(str): pass
class XLDate(int): pass


def test_createTypes():
    XeBumpable = newImmutableRecordType('XeBumpable', XeTradeId=XeHandle, XeVdId=XeHandle, Portfolio=str)

    XeAsset = newImmutableRecordType('XeAsset')
    XeTrade = joinImmutableRecordTypes(
        newImmutableRecordType(TradeDt=dt.date | XLDate | Missing, XeAsset=XeHandle),
        XeBumpable
    )
    XeVd = newImmutableRecordType('XeVd', )
    XeBond = newImmutableRecordType('XeBond', Cpn=float)
    XeBondFut = newImmutableRecordType('XeBondFut', FirstDeliveryDt=XLDate | dt.date)

    XeBondTrade = joinImmutableRecordTypes(
        XeTrade,
        newImmutableRecordType(SettleDt=dt.date | XLDate | Missing)
    )
    return XeBumpable, XeAsset, XeTrade, XeVd, XeBond, XeBondFut, XeBondTrade


def test_construction():
    XeBumpable, XeAsset, XeTrade, XeVd, XeBond, XeBondFut, XeBondTrade = test_createTypes()
    dbrFeb34 = XeBond()
    trade1 = XeBondTrade(XeAsset=dbrFeb34, TradeDt=dt.date.today())
    vd1 = XeVd()


def test_immutability():
    XeBumpable, XeAsset, XeTrade, XeVd, XeBond, XeBondFut, XeBondTrade = test_createTypes()
    trade = XeBondTrade(TradeDt=dt.date.today())
    trade.SettleDt = 't+2'
    with assertRaises(AttributeError):
        trade.SettleDt = 't+1'
    with assertRaises(AttributeError):
        del trade.SettleDt


def test_fits():
    XeBumpable, XeAsset, XeTrade, XeVd, XeBond, XeBondFut, XeBondTrade = test_createTypes()

    assert fitsWithin(XeTrade, XeBumpable)
    assert fitsWithin(XeTrade, Union[XeBumpable, int])
    assert fitsWithin(XeTrade, Union[XeBumpable, int])
    assert fitsWithin(Union[XeBumpable, int], Union[XeBumpable, int])
    assert not fitsWithin(Union[XeBumpable, int], XeTrade)

    assert fitsWithin(RBBondTrade, RBTradeVdPair)


@immutable_record
class RBTradeVdPair(ImmutableRecord):
    XeTrade: str
    XeVd: str | Missing         # some trades don't need a companion valuation data
    Portfolio: str | Missing


@immutable_record
class RBTrade(ImmutableRecord):
    TradeDt: int                # XLDate
    FeedId: str
    TimeId: str
    Size: float
    Desc: str
    AssetType: str
    # asset specific details ideally would be inserted here - is this possible with dataclasses?
    Price: float | Missing
    Portfolio: str | Missing
    TradeId: int | Missing
    XeAsset: str
    XeTrade: str
    XeVd: str | Missing


@immutable_record
class RBBondTrade(RBTrade):
    Isin: str
    Alias1: str
    Alias2: str
    MlpCode: str
    Cusip: str
    Ccy: str
    Issuer: str
    MaturityDt: int             # XLDate
    IssueDt: int | Missing      # XLDate
    DatedDt: int | Missing      # XLDate
    FirstCpnDt: int             # XLDate
    Mkt: str
    Cpn: float
    Freq: int
    IsFloater: bool
    IsLinker: bool
    XeConv: str
    XeCalendar: str
    BondPricingCurve: str
    SettleDt: int               # XLDate
    Z: float



def main():
    test_createTypes()
    test_fits()
    test_construction()
    test_immutability()


if __name__ == '__main__':
    main()
    print('passed')
