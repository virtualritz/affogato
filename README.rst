# Affogato

Affogato is a plugin for Autodesk's Softimage XSI 3D animation package. It links XSI with high-end offline renderers.

It allows a great level of customization to fit into contemporary VFX pipelines. Currently Affogato only supports RenderMan-compliant renderers.

## History

In 2005 Rising Sun Pictures (RSP) won a bid to do the main character of the 2006 summer blockbuster "Charlotte's Web". Charlotte, the spider.

The team at RSP wanted to use Softimage XSI (later Autodesk XSI, then discontinued). There was no bridge availabe from XSI to any standalone offline renderers RSP wanted to use.
When the project started it was planned to use Nvidia's Gelato but being GPU-based and still slow and lacking required feartures, compared to CPU-based 3Delight, it was decided to go with the latter.

I wrote the code alone. Being written during production means some areas are not clean, some features are incomplete and there is dead code.

## Code Quality

This is a snapshot of the repository when the project ended. RSP abandoned XSI afterwards so no cleanup or further development happened to the code.

I guess I am saying: don't be too judgy about code quality. It is different to write software that needs to work in six months from now than software that needs to work asap and then pretty much all of the time while users pile on features that they need yesterday.

All tests we had were image-based. I.e. there were no unit or functional tests. Images had to look the same before/after a feature was added or a bug was fixed since images are the only thing that matters at the end, in VFX production. One pixel difference and code goes into review/you can't commit.
